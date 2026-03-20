# WiFi Tester – Lessons Learned

## HTTPS on ESP32 requires `WiFiClientSecure`

Plain `HTTPClient::begin(url)` silently fails for `https://` URLs. You must pass a `WiFiClientSecure` instance:

```cpp
WiFiClientSecure client;
client.setCACert(POLL_CA_CERT);
http.begin(client, host, 443, path, true);
```

Using the explicit `host / port / path` form of `begin()` is more reliable than passing the full URL string.

## `setInsecure()` still requires a successful TLS handshake

`setInsecure()` skips certificate verification but does not bypass the TLS handshake itself. If the handshake fails (cipher mismatch, protocol version), you still get a connection error. Always prefer `setCACert()` with the correct root CA — it also gives mbedTLS a proper trust anchor to negotiate against.

## "Connection refused" from HTTPClient can mean a TLS failure

`HTTPClient` maps a failed `client->connect()` to `HTTPC_ERROR_CONNECTION_REFUSED (-1)`. This error can mean TLS handshake failure, not just TCP refusal. Use a plain TCP probe first to distinguish the two:

```cpp
WiFiClient probe;
probe.connect(host, port);   // tests TCP only
```

If TCP succeeds but HTTPClient still fails, the issue is TLS. Use `secureClient.lastError()` to get the actual SSL error string.

## ECDSA certificates need ECDSA cipher suites in nginx

Let's Encrypt's newer intermediates (E5, E6, E7, E8) issue **ECDSA certificates**, not RSA. A common nginx cipher list of `ECDHE-RSA-*:DHE-RSA-*` is RSA-only and will cause TLS 1.2 `handshake_failure` with ECDSA certs. Fix: prepend the ECDSA variants:

```nginx
ssl_ciphers ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA512:...;
```

TLS 1.3 is unaffected because its cipher suite format does not encode the authentication method.

## ESP32 Arduino core only supports TLS 1.2 by default

The mbedTLS configuration baked into the ESP32 Arduino core negotiates TLS 1.2. If a server is configured for TLS 1.3 only the handshake fails. Ensure your server has `ssl_protocols TLSv1.2 TLSv1.3;`.

## ADC2 pins are unavailable while WiFi is active

Any sketch that combines WiFi with analog reads must use **ADC1 pins (GPIO 32–39)**. ADC2 pins (GPIO 0, 2, 4, 12–15, 25–27) return garbage while the WiFi radio is on.

## NTP is required for valid Unix timestamps

The ESP32 has no RTC. `time()` returns values near 0 until NTP syncs. Call `configTime()` after WiFi connects and wait for a plausible epoch value before using timestamps in requests.

## secrets.h is the Arduino equivalent of .env

Arduino IDE has no native `.env` support. The standard pattern is a `secrets.h` header in the sketch folder, `#include`d at compile time. Exclude it from git with `**/secrets.h` in `.gitignore` and commit a `secrets.h.example` template instead.
