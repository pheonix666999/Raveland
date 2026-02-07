#!/usr/bin/env bash
set -euo pipefail

# Creates a double-click installer (.pkg) that installs:
# - /Library/Audio/Plug-Ins/VST3/RaveLand.vst3
# - /Library/Audio/Plug-Ins/Components/RaveLand.component
#
# Usage:
#   bash scripts/macos_make_installer_pkg.sh [artefacts_dir] [out_dir]
#
# Optional env vars:
#   APPLE_INSTALLER_IDENTITY="Developer ID Installer: Company (TEAMID)"   (sign the .pkg)
#   APPLE_NOTARY_KEY_ID / APPLE_NOTARY_ISSUER_ID / APPLE_NOTARY_KEY_PATH  (notarize + staple the signed .pkg)
#
# This script must be run on macOS.

artefacts_dir="${1:-build/Raveland_artefacts/Release}"
out_dir="${2:-dist}"
require_notarization="${REQUIRE_NOTARIZATION:-0}"

vst3_src="${artefacts_dir}/VST3/RaveLand.vst3"
au_src="${artefacts_dir}/AU/RaveLand.component"

if [[ ! -d "${vst3_src}" && ! -d "${au_src}" ]]; then
  echo "ERROR: No macOS VST3/AU found under '${artefacts_dir}'." >&2
  echo "       Expected '${vst3_src}' and/or '${au_src}'." >&2
  exit 2
fi

mkdir -p "${out_dir}"

work_dir="$(mktemp -d -t raveland_pkg.XXXXXX)"
trap 'rm -rf "${work_dir}"' EXIT

pkgroot="${work_dir}/pkgroot"
mkdir -p "${pkgroot}/Library/Audio/Plug-Ins/VST3"
mkdir -p "${pkgroot}/Library/Audio/Plug-Ins/Components"

if [[ -d "${vst3_src}" ]]; then
  cp -R "${vst3_src}" "${pkgroot}/Library/Audio/Plug-Ins/VST3/"
fi
if [[ -d "${au_src}" ]]; then
  cp -R "${au_src}" "${pkgroot}/Library/Audio/Plug-Ins/Components/"
fi

pkg_id="com.nsaudio.raveland.installer"
pkg_version="${PKG_VERSION:-0.1.0}"

unsigned_pkg="${work_dir}/RaveLand-Installer-unsigned.pkg"
pkgbuild \
  --root "${pkgroot}" \
  --identifier "${pkg_id}" \
  --version "${pkg_version}" \
  --install-location "/" \
  "${unsigned_pkg}"

out_unsigned_pkg="${out_dir}/RaveLand-Installer-unsigned.pkg"
rm -f "${out_unsigned_pkg}"
cp -f "${unsigned_pkg}" "${out_unsigned_pkg}"
echo "Wrote: ${out_unsigned_pkg}"

if [[ -n "${APPLE_INSTALLER_IDENTITY:-}" ]]; then
  final_pkg="${out_dir}/RaveLand-Installer.pkg"
  rm -f "${final_pkg}"

  productsign --sign "${APPLE_INSTALLER_IDENTITY}" "${unsigned_pkg}" "${final_pkg}"
  echo "Wrote: ${final_pkg}"

  key_id="${APPLE_NOTARY_KEY_ID:-}"
  issuer_id="${APPLE_NOTARY_ISSUER_ID:-}"
  key_path="${APPLE_NOTARY_KEY_PATH:-}"

  if [[ -n "${key_id}" && -n "${issuer_id}" && -n "${key_path}" ]]; then
    if [[ ! -f "${key_path}" ]]; then
      echo "ERROR: APPLE_NOTARY_KEY_PATH does not exist: ${key_path}" >&2
      exit 4
    fi

    echo "Notarizing installer package..."
    xcrun notarytool submit "${final_pkg}" --key-id "${key_id}" --issuer "${issuer_id}" --key "${key_path}" --wait
    xcrun stapler staple "${final_pkg}"
    xcrun stapler validate "${final_pkg}"
    echo "Notarized + stapled: ${final_pkg}"
  else
    if [[ "${require_notarization}" != "0" ]]; then
      echo "ERROR: REQUIRE_NOTARIZATION is set, but notarization creds are missing for the .pkg." >&2
      exit 5
    fi
    echo "NOTE: Notarization creds not set; produced a signed but NOT notarized .pkg."
  fi
fi
