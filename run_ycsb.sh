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
repeat_num="${REPEAT_NUM:-3}"
threads=(${THREADS_OVERRIDE:-1 2 4 8})
db_names=("splitlsm" "unifiedlsm")
workload_dir="${1:-${this_dir}/workloads}"
log_file="${2:-${this_dir}/ycsbc.output}"
db_root="${DB_ROOT:-/tmp/ycsbc_lsm}"
workload_glob="${WORKLOAD_GLOB:-workload_*.spec}"

splitlsm_opts=(
  "-p splitlsm.buffer_size=${SPLITLSM_BUFFER_SIZE:-8388608}"
  "-p splitlsm.collect_size=${SPLITLSM_COLLECT_SIZE:-134217728}"
  "-p splitlsm.collect_threshold=${SPLITLSM_COLLECT_THRESHOLD:-134217728}"
  "-p splitlsm.punch_hole_threshold=${SPLITLSM_PUNCH_HOLE_THRESHOLD:-134217728}"
  "-p splitlsm.write_buffer_size=${SPLITLSM_WRITE_BUFFER_SIZE:-67108864}"
)

unifiedlsm_opts=(
  "-p unifiedlsm.write_buffer_size=${UNIFIEDLSM_WRITE_BUFFER_SIZE:-67108864}"
)

run_opts_for_db() {
  local db="$1"
  if [[ "$db" == "splitlsm" ]]; then
    printf "%s " "${splitlsm_opts[@]}"
  else
    printf "%s " "${unifiedlsm_opts[@]}"
  fi
}

if ! ls "${workload_dir}"/${workload_glob} >/dev/null 2>&1; then
  echo "No workload specs matching ${workload_glob} found in ${workload_dir}. Run ./workloads.sh first." >&2
  exit 1
fi

touch "$log_file"

for spec in "${workload_dir}"/${workload_glob}; do
  [[ -f "$spec" ]] || continue
  base="$(basename "${spec%.spec}")"

  for db in "${db_names[@]}"; do
    db_path="${db_root}/${db}_${base}"
    echo "Preparing ${db} database at ${db_path} for ${base}"
    rm -rf "${db_path}"
    mkdir -p "${db_path}"

    echo "Loading ${db} with ${spec}"
    "$bin_path" -db "$db" -threads 1 -load -P "$spec" \
      -p "${db}.dbpath=${db_path}" \
      $(run_opts_for_db "$db") \
      2>>"$log_file"

    for tn in "${threads[@]}"; do
      for ((i=1; i<=repeat_num; ++i)); do
        echo "Running ${db} workload ${base} with ${tn} threads (run ${i})"
        "$bin_path" -db "$db" -threads "$tn" -run -P "$spec" \
          -p "${db}.dbpath=${db_path}" \
          $(run_opts_for_db "$db") \
          2>>"$log_file"
      done
    done
  done
done

echo "Done. Results appended to ${log_file}"
