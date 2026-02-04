#!/usr/bin/env bash
set -euo pipefail

# Creates an unsigned zip containing the built macOS bundles (Standalone/VST3/AU).
# This is useful for internal testing and as a fallback when signing/notarization
# is not configured in CI.
#
# Usage:
#   bash scripts/macos_package_unsigned.sh [artefacts_dir] [out_dir]
#
# Output:
#   dist/RaveLand-macOS-unsigned.zip
#
# This script must be run on macOS.

artefacts_dir="${1:-build/Raveland_artefacts/Release}"
out_dir="${2:-dist}"

mkdir -p "${out_dir}"

vst3_path="${artefacts_dir}/VST3/RaveLand.vst3"
au_path="${artefacts_dir}/AU/RaveLand.component"
app_path="${artefacts_dir}/Standalone/RaveLand.app"

targets=()
[[ -d "${vst3_path}" ]] && targets+=("${vst3_path}")
[[ -d "${au_path}" ]] && targets+=("${au_path}")
[[ -d "${app_path}" ]] && targets+=("${app_path}")

if [[ "${#targets[@]}" -eq 0 ]]; then
  echo "ERROR: No macOS artefacts found under '${artefacts_dir}' (expected VST3/AU/Standalone)." >&2
  exit 3
fi

dist_zip="${out_dir}/RaveLand-macOS-unsigned.zip"
rm -f "${dist_zip}"

stage_dir="$(mktemp -d -t raveland_dist.XXXXXX)"
trap 'rm -rf "${stage_dir}"' EXIT

for t in "${targets[@]}"; do
  cp -R "${t}" "${stage_dir}/"
done

/usr/bin/ditto -c -k --sequesterRsrc --keepParent "${stage_dir}" "${dist_zip}"
echo "Wrote: ${dist_zip}"

