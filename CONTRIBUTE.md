# Contributing to cpp-hub

Thank you for your interest in **cpp-hub**.

> **Please Note:** As stated in the README, this repository is a conceptual reference implementation ("what could have been") and is currently in a highly volatile state of construction.

While community engagement is appreciated, the capacity to accept contributions is limited by the project's educational and experimental nature. Please review the guidelines below to ensure efficiency.

---

## Project Status & Philosophy

This project is intended to serve as a starting point or a reference design for a C++ project template hub, rather than a maintained, production-grade product.

* **Volatility:** The codebase may undergo significant rewrites or breaking changes without notice.
* **Maintenance:** There is no guarantee of long-term maintenance or backwards compatibility.

---

## How to Contribute

### 1. Reporting Issues

If a bug is encountered or a suggestion arises, please open an Issue. However, please be advised that issues regarding feature requests or edge-case bugs may be deprioritized in favor of core architectural work.

When filing an issue, please clearly state:
* Whether the report concerns a compilation error or a runtime logic error.
* The version of the C++ compiler and CMake being used.

### 2. Pull Requests

**Please do not open a Pull Request for new features without prior discussion.**

Due to the unstable nature of the repository, unsolicited PRs adding significant logic or dependencies are likely to be closed. PRs are primarily accepted if they:

* Fix critical compilation errors on standard platforms (Linux, macOS, Windows).
* Correct documentation or typos.
* Fix clear logic errors in existing features.

If a major change is proposed, please open an Issue tagged `[Proposal]` first to allow for discussion regarding alignment with the project's reference goals.

### 3. Development Environment

If submitting code, please ensure:

* **Language Standard:** Code must compile with `C++20`.
* **Formatting:** An attempt should be made to match the existing coding style (Standard C++ naming conventions).
* **Dependencies:** New dependencies (beyond `nlohmann/json`) should not be introduced unless absolutely necessary and agreed upon.

---

## License & Attribution

By contributing to cpp-hub, you agree that your contributions will be licensed under the same license as the project root (GPLv3).
