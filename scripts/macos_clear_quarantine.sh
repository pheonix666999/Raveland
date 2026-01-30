#!/usr/bin/env bash
set -euo pipefail

paths=("$@")
if [[ "${#paths[@]}" -eq 0 ]]; then
  paths=(
    "/Library/Audio/Plug-Ins/VST3/RaveLand.vst3"
    "/Library/Audio/Plug-Ins/Components/RaveLand.component"
    "$HOME/Library/Audio/Plug-Ins/VST3/RaveLand.vst3"
    "$HOME/Library/Audio/Plug-Ins/Components/RaveLand.component"
  )
fi

did_any=0
for p in "${paths[@]}"; do
  if [[ -e "${p}" ]]; then
    echo "Clearing quarantine: ${p}"
    xattr -dr com.apple.quarantine "${p}" || true
    did_any=1
  fi
done

if [[ "${did_any}" -eq 0 ]]; then
  echo "No matching plugin bundles found; pass paths explicitly." >&2
  exit 1
fi

