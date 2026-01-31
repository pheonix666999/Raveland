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
#
# This script must be run on macOS.

artefacts_dir="${1:-build/Raveland_artefacts/Release}"
out_dir="${2:-dist}"

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

final_pkg="${out_dir}/RaveLand-Installer.pkg"
rm -f "${final_pkg}"

if [[ -n "${APPLE_INSTALLER_IDENTITY:-}" ]]; then
  productsign --sign "${APPLE_INSTALLER_IDENTITY}" "${unsigned_pkg}" "${final_pkg}"
else
  cp -f "${unsigned_pkg}" "${final_pkg}"
fi

echo "Wrote: ${final_pkg}"

