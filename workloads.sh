#!/usr/bin/env bash
set -euo pipefail

this_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
workload_dir="${1:-${this_dir}/workloads}"

mkdir -p "${workload_dir}"

recordcount="${RECORDCOUNT:-100000}"
operationcount="${OPERATIONCOUNT:-100000}"
distribution="${REQUEST_DISTRIBUTION:-zipfian}"

emit_workload() {
  local name="$1"
  local read_p="$2"
  local update_p="$3"
  local delete_p="$4"
  local dist="${5:-$distribution}"
  local description="$6"

  cat >"${workload_dir}/${name}.spec" <<EOF
# ${description}
recordcount=${recordcount}
operationcount=${operationcount}
workload=com.yahoo.ycsb.workloads.CoreWorkload

fieldcount=1
fieldlength=100
field_len_dist=constant
readallfields=true
writeallfields=true

readproportion=${read_p}
updateproportion=${update_p}
deleteproportion=${delete_p}
insertproportion=0
scanproportion=0
readmodifywriteproportion=0

requestdistribution=${dist}
EOF
}

emit_workload "workload_put" \
  "0" "1" "0" "${distribution}" \
  "Update-only workload (Put)"

emit_workload "workload_get" \
  "1" "0" "0" "${distribution}" \
  "Read-only workload (Get)"

emit_workload "workload_delete" \
  "0" "0" "1" "${distribution}" \
  "Delete-only workload"

emit_workload "workload_mixed_read_heavy" \
  "0.7" "0.2" "0.1" "${distribution}" \
  "Mixed workload: 70% read, 20% update, 10% delete"

emit_workload "workload_mixed_balanced" \
  "0.4" "0.4" "0.2" "${distribution}" \
  "Mixed workload: 40% read, 40% update, 20% delete"

emit_workload "workload_mixed_write_delete" \
  "0.1" "0.5" "0.4" "${distribution}" \
  "Mixed workload: 10% read, 50% update, 40% delete"

echo "Wrote workload specs to ${workload_dir}"
