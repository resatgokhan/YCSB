#!/usr/bin/env bash
set -euo pipefail

this_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
bin_path="${this_dir}/ycsbc"
if [[ ! -x "$bin_path" && -x "${this_dir}/build/ycsbc" ]]; then
  bin_path="${this_dir}/build/ycsbc"
fi
if [[ ! -x "$bin_path" ]]; then
  echo "ycsbc binary not found. Build with 'cmake --build build' first." >&2
  exit 1
fi

db_names=(${DB_NAMES_OVERRIDE:-splitlsm unifiedlsm})
threads=(${THREADS_OVERRIDE:-1})
workload_dir="${WORKLOAD_DIR:-${this_dir}/workloads}"
log_file="${LOG_FILE:-${this_dir}/ycsbc.output}"
db_root="${DB_ROOT:-/tmp/ycsbc_lsm}"
workload_globs=(
  "${WORKLOAD_GLOB_A:-workloada_*.spec}"
  "${WORKLOAD_GLOB_B:-workloadb_*.spec}"
  "${WORKLOAD_GLOB_C:-workloadc_*.spec}"
  "${WORKLOAD_GLOB_D:-workloadd_*.spec}"
  "${WORKLOAD_GLOB_E:-workloade_*.spec}"
  "${WORKLOAD_GLOB_F:-workloadf_*.spec}"
)

found_spec=0
for glob in "${workload_globs[@]}"; do
  for spec in "${workload_dir}"/${glob}; do
    if [[ -f "$spec" ]]; then
      found_spec=1
      break 2
    fi
  done
done

if [[ "$found_spec" -eq 0 ]]; then
  echo "No workload specs matching A-F globs found in ${workload_dir}. Ensure workloads/*.spec exist." >&2
  exit 1
fi

touch "$log_file"

has_result() {
  local db="$1"
  local spec="$2"
  local tn="$3"
  local needle="${db}"$'\t'"${spec}"$'\t'"${tn}"$'\t'
  grep -qF "$needle" "$log_file"
}

for glob in "${workload_globs[@]}"; do
  for spec in "${workload_dir}"/${glob}; do
    [[ -f "$spec" ]] || continue
    base="$(basename "${spec%.spec}")"

    for db in "${db_names[@]}"; do
      missing=0
      for tn in "${threads[@]}"; do
        if ! has_result "$db" "$spec" "$tn"; then
          missing=1
          break
        fi
      done
      if [[ "$missing" -eq 0 ]]; then
        echo "Skipping ${db} ${base} (all threads already in ${log_file})"
        continue
      fi
      db_path="${db_root}/${db}_${base}"

    echo "Preparing ${db} database at ${db_path} for ${base}"
    rm -rf "${db_path}"
    mkdir -p "${db_path}"

    extra_opts=()
    if [[ "$db" == "splitlsm" ]]; then
      extra_opts+=("-p" "splitlsm.enable_gc=false")
    fi

    echo "Loading ${db} with ${spec}"
    "$bin_path" -db "$db" -threads 1 -load -P "$spec" \
      -p "${db}.dbpath=${db_path}" \
      "${extra_opts[@]}" \
      2>>"$log_file"

      for tn in "${threads[@]}"; do
        if has_result "$db" "$spec" "$tn"; then
          echo "Skipping ${db} workload ${base} with ${tn} threads (already in ${log_file})"
          continue
        fi
        echo "Running ${db} workload ${base} with ${tn} threads"
        "$bin_path" -db "$db" -threads "$tn" -run -P "$spec" \
          -p "${db}.dbpath=${db_path}" \
          "${extra_opts[@]}" \
          2>>"$log_file"
      done
    done
  done
done

echo "Done. Results appended to ${log_file}"
