#!/usr/bin/env bash
set -euo pipefail

artefacts_dir="${1:-build/Raveland_artefacts/Release}"
out_dir="${2:-dist}"

identity="${APPLE_SIGNING_IDENTITY:-}"
if [[ -z "${identity}" ]]; then
  echo "ERROR: APPLE_SIGNING_IDENTITY is required (e.g. 'Developer ID Application: Your Company (TEAMID)')" >&2
  exit 2
fi

key_id="${APPLE_NOTARY_KEY_ID:-}"
issuer_id="${APPLE_NOTARY_ISSUER_ID:-}"
key_path="${APPLE_NOTARY_KEY_PATH:-}"

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

echo "Signing:"
for t in "${targets[@]}"; do
  echo "  - ${t}"
  # JUCE bundles can contain nested frameworks/binaries. --deep signs nested code as well.
  codesign --force --timestamp --options runtime --deep --sign "${identity}" "${t}"
done

echo "Verifying signatures:"
for t in "${targets[@]}"; do
  codesign --verify --deep --strict --verbose=2 "${t}"
done

if [[ -z "${key_id}" || -z "${issuer_id}" || -z "${key_path}" ]]; then
  cat >&2 <<'EOF'
WARNING: Notarization not configured (APPLE_NOTARY_KEY_ID / APPLE_NOTARY_ISSUER_ID / APPLE_NOTARY_KEY_PATH).
         The artefacts are signed, but Gatekeeper may still show "damaged" for users who downloaded them.
         Configure notarization to eliminate that for end users.
EOF
else
  if [[ ! -f "${key_path}" ]]; then
    echo "ERROR: APPLE_NOTARY_KEY_PATH does not exist: ${key_path}" >&2
    exit 4
  fi

  tmp_zip="$(mktemp -t raveland_notary.XXXXXX).zip"
  tmp_dir="$(mktemp -d -t raveland_payload.XXXXXX)"
  trap 'rm -f "${tmp_zip}"; rm -rf "${tmp_dir}"' EXIT

  for t in "${targets[@]}"; do
    cp -R "${t}" "${tmp_dir}/"
  done

  # Notarytool prefers a single zip payload.
  /usr/bin/ditto -c -k --sequesterRsrc --keepParent "${tmp_dir}" "${tmp_zip}"

  echo "Submitting to Apple notarization service..."
  xcrun notarytool submit "${tmp_zip}" --key-id "${key_id}" --issuer "${issuer_id}" --key "${key_path}" --wait

  echo "Stapling notarization tickets:"
  for t in "${targets[@]}"; do
    xcrun stapler staple "${t}"
    xcrun stapler validate "${t}"
  done
fi

dist_zip="${out_dir}/RaveLand-macOS.zip"
rm -f "${dist_zip}"

stage_dir="$(mktemp -d -t raveland_dist.XXXXXX)"
trap 'rm -rf "${stage_dir}"' EXIT
for t in "${targets[@]}"; do
  cp -R "${t}" "${stage_dir}/"
done

/usr/bin/ditto -c -k --sequesterRsrc --keepParent "${stage_dir}" "${dist_zip}"
echo "Wrote: ${dist_zip}"

