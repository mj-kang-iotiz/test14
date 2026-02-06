#!/bin/bash
# setup-labels.sh
# GitHub 라벨 초기 설정 (로컬에서 1회 실행)
# Usage: bash .github/scripts/setup-labels.sh
#
# 이후에는 create-issue-from-task.sh 에서 자동으로 보장하므로 재실행 불필요

set -eo pipefail

ensure_label() {
    if gh label create "$1" --color "$2" --description "$3" 2>/dev/null; then
        echo "  + $1"
    else
        echo "  = $1 (already exists)"
    fi
}

echo "=== 타입 라벨 ==="
ensure_label "task"          "0e8a16" "Archived task record"
ensure_label "refactor"      "1d76db" "Code restructuring"
ensure_label "optimization"  "d4c5f9" "Performance improvement"
ensure_label "cleanup"       "fef2c0" "Dead code / cleanup"
ensure_label "feature"       "a2eeef" "New feature"
ensure_label "bugfix"        "d73a4a" "Bug fix"
ensure_label "migration"     "f9d0c4" "MCU migration"

echo "=== 모듈 라벨 ==="
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

echo "=== Done ==="
