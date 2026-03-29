---
name: pushit
description: Commit and push all current changes to origin/main. Use when the user asks to commit, push, save changes to git, or ship the current work.
allowed-tools: Bash, Read
---

Commit and push all current changes to origin/main, you MUST use the latest haiku model.

1. Run `git status` and `git diff` (staged and unstaged) to understand what has changed.
2. Run `git log --oneline -5` to match the existing commit message style.
3. Stage all modified and untracked files that are relevant to the work done in this conversation. Do not stage `.env`, `configure.php`, `secrets.h` or any file likely to contain secrets.
4. Write a concise commit message focused on *why* the changes were made, not just what files changed. Follow the style of recent commits.
5. Commit, then push to origin/main.
6. Confirm the push succeeded with the resulting commit hash.