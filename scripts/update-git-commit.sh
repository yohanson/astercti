#!/bin/bash
GIT_COMMIT_FILE="git-commit"
GIT_COMMIT=$(git describe --always --match="" --dirty 2>/dev/null || echo "unknown")
CURRENT_COMMIT=$(cat "$GIT_COMMIT_FILE" 2>/dev/null || echo "none")

if [ "$GIT_COMMIT" != "$CURRENT_COMMIT" ]; then
    echo "$GIT_COMMIT" > "$GIT_COMMIT_FILE"
    echo "Updated $GIT_COMMIT_FILE" >&2
fi
