#!/bin/bash
# create-issue-from-task.sh
# PR merge 시 tasks/archive/ 에 추가된 .md 파일을 GitHub Issue로 생성 (closed)
#
# 필요 환경변수:
#   GH_TOKEN            - GitHub 토큰
#   PR_NUMBER           - 머지된 PR 번호
#   GITHUB_REPOSITORY   - owner/repo

set -eo pipefail

PR_NUMBER="${PR_NUMBER:?PR_NUMBER required}"
REPO="${GITHUB_REPOSITORY:?GITHUB_REPOSITORY required}"

#==== 라벨 보장 (없으면 생성, 있으면 무시) ====

ensure_label() {
    gh label create "$1" --color "$2" --description "$3" --repo "$REPO" 2>/dev/null || true
}

# 타입 라벨
ensure_label "task"          "0e8a16" "Archived task record"
ensure_label "refactor"      "1d76db" "Code restructuring"
ensure_label "optimization"  "d4c5f9" "Performance improvement"
ensure_label "cleanup"       "fef2c0" "Dead code / cleanup"
ensure_label "feature"       "a2eeef" "New feature"
ensure_label "bugfix"        "d73a4a" "Bug fix"
ensure_label "migration"     "f9d0c4" "MCU migration"

# 모듈 라벨
ensure_label "mod:gps"       "006b75" "GPS module"
ensure_label "mod:ble"       "1c71d8" "BLE module"
ensure_label "mod:lora"      "5319e7" "LoRa module"
ensure_label "mod:gsm"       "b60205" "GSM module"
ensure_label "mod:ntrip"     "e99695" "NTRIP module"
ensure_label "mod:rs485"     "fbca04" "RS485 module"
ensure_label "mod:rs232"     "d93f0b" "RS232 module"
ensure_label "mod:fdcan"     "c2e0c6" "FDCAN module"
ensure_label "mod:led"       "bfdadc" "LED module"
ensure_label "mod:system"    "ededed" "System / core"

#==== PR에서 archive된 태스크 파일 찾기 ====

FILES=$(gh api "repos/${REPO}/pulls/${PR_NUMBER}/files" \
    --jq '.[] | select(.filename | startswith("tasks/archive/")) | select(.filename | endswith(".md")) | select(.status != "removed") | .filename')

if [ -z "$FILES" ]; then
    echo "No archived tasks in PR #${PR_NUMBER}"
    exit 0
fi

#==== 각 파일에서 이슈 생성 ====

while IFS= read -r file; do
    [ -z "$file" ] && continue
    [ ! -f "$file" ] && echo "SKIP: $file not found" && continue

    echo "--- Processing: $file ---"

    raw=$(cat "$file")

    # ---- Front-matter 파싱 ----
    type_label=""
    mod_label_str=""

    first_line=$(head -1 "$file")

    if [ "$first_line" = "---" ]; then
        # front-matter 추출 (첫 --- ~ 두번째 --- 사이)
        fm=$(awk '/^---$/{n++; next} n==1{print} n>=2{exit}' "$file")
        # body 추출 (두번째 --- 이후)
        body=$(awk 'BEGIN{n=0} /^---$/{n++; next} n>=2{print}' "$file")

        # type
        type_label=$(echo "$fm" | awk -F': *' '/^type:/{print $2; exit}' | tr -d '[:space:]')

        # modules: [gps, ble] → "mod:gps mod:ble"
        mod_raw=$(echo "$fm" | sed -n 's/^modules: *\[//p' | sed 's/\].*//')
        if [ -n "$mod_raw" ]; then
            IFS=',' read -ra mods <<< "$mod_raw"
            for m in "${mods[@]}"; do
                m=$(echo "$m" | tr -d '[:space:]')
                [ -n "$m" ] && mod_label_str="${mod_label_str:+$mod_label_str }mod:$m"
            done
        fi
    else
        body="$raw"
    fi

    # ---- 제목 추출 ----
    title=$(echo "$body" | grep -m1 '^# ' | sed 's/^# //')
    [ -z "$title" ] && title=$(basename "$file" .md | tr '_-' ' ')

    # body에서 제목 줄 제거 + 앞쪽 빈줄 제거
    body=$(echo "$body" | sed '0,/^# /{/^# /d}' | sed '/./,$!d')

    # ---- 모듈 자동 감지 (front-matter에 없을 때) ----
    if [ -z "$mod_label_str" ]; then
        echo "$raw" | grep -q 'lib/gps/\|app/gps/'     && mod_label_str="${mod_label_str:+$mod_label_str }mod:gps"
        echo "$raw" | grep -q 'lib/ble/\|app/ble/'     && mod_label_str="${mod_label_str:+$mod_label_str }mod:ble"
        echo "$raw" | grep -q 'lib/gsm/\|app/gsm/'     && mod_label_str="${mod_label_str:+$mod_label_str }mod:gsm"
        echo "$raw" | grep -q 'lib/lora/\|app/lora/'   && mod_label_str="${mod_label_str:+$mod_label_str }mod:lora"
        echo "$raw" | grep -q 'lib/rs485/\|app/rs485/' && mod_label_str="${mod_label_str:+$mod_label_str }mod:rs485"
        echo "$raw" | grep -q 'lib/rs232/\|app/rs232/' && mod_label_str="${mod_label_str:+$mod_label_str }mod:rs232"
        echo "$raw" | grep -q 'lib/fdcan/\|app/fdcan/' && mod_label_str="${mod_label_str:+$mod_label_str }mod:fdcan"
        echo "$raw" | grep -q 'ntrip'                   && mod_label_str="${mod_label_str:+$mod_label_str }mod:ntrip"
    fi

    # ---- 라벨 문자열 빌드 (쉼표 구분) ----
    label_str="task"
    [ -n "$type_label" ] && label_str="${label_str},${type_label}"
    for ml in $mod_label_str; do
        label_str="${label_str},${ml}"
    done

    # ---- Issue body 작성 ----
    body_file=$(mktemp)
    cat > "$body_file" <<EOF
${body}

---
*Completed in #${PR_NUMBER} | Source: \`${file}\`*
EOF

    # ---- Issue 생성 + 즉시 닫기 ----
    issue_url=$(gh issue create \
        --title "$title" \
        --body-file "$body_file" \
        --label "$label_str" \
        --repo "$REPO")

    echo "Created: $issue_url"

    issue_num=$(echo "$issue_url" | grep -oE '[0-9]+$')
    gh issue close "$issue_num" --repo "$REPO"
    echo "Closed: #$issue_num"

    rm -f "$body_file"

done <<< "$FILES"

echo "=== Done ==="
