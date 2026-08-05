#!/usr/bin/env bash

set -u

PROGRAM=${1:-./codexion}
REPEAT=${REPEAT:-10}
VERBOSE=${VERBOSE:-0}
FAIL_DIR=${FAIL_DIR:-codexion_test_failures}

total=0
passed=0
failed=0
unexpected_burnout=0
missing_burnout=0
id_failures=0
malformed_failures=0
timeout_failures=0
unexpected_nonzero=0
expected_rejection_failed=0
segfault_failures=0
abort_failures=0
other_signal_failures=0
compile_count_failures=0
output_after_burnout_failures=0
multiple_burnout_failures=0
dongle_failures=0
failure_serial=0
run_id=$$

if [[ ! -x "$PROGRAM" ]]; then
	printf 'Error: executable not found or not executable: %s\n' "$PROGRAM" >&2
	exit 1
fi
if [[ ! "$REPEAT" =~ ^[1-9][0-9]*$ ]]; then
	printf 'Error: REPEAT must be a positive integer.\n' >&2
	exit 1
fi
if command -v timeout >/dev/null 2>&1; then
	TIMEOUT_CMD=timeout
else
	TIMEOUT_CMD=
	printf 'Warning: timeout command is unavailable; deadlock protection is disabled.\n' >&2
fi

mkdir -p "$FAIL_DIR" || exit 1
tmp_dir=$(mktemp -d "${TMPDIR:-/tmp}/codexion-test.XXXXXX") || exit 1
trap 'rm -rf "$tmp_dir"' EXIT HUP INT TERM
PROGRAM_RUN="$tmp_dir/codexion-under-test"
if ! cp "$PROGRAM" "$PROGRAM_RUN" || ! chmod +x "$PROGRAM_RUN"; then
	printf 'Error: could not create a stable executable snapshot.\n' >&2
	exit 1
fi

calculate_timeout()
{
	local coders=$1 compiles=$2 compile_ms=$3 debug_ms=$4 refactor_ms=$5 cooldown_ms=$6
	local estimate

	estimate=$((coders * compiles * (compile_ms + debug_ms + refactor_ms + cooldown_ms) * 3))
	estimate=$(((estimate + 999) / 1000 + 2))
	((estimate < 5)) && estimate=5
	printf '%s' "$estimate"
}

compile_counts()
{
	local output=$1 coders=$2 i
	for ((i = 1; i <= coders; i++)); do
		awk -v id="$i" '$2 == id && $3 == "is" && $4 == "compiling" {n++} END {print n + 0}' "$output"
	done
}

validate_output()
{
	local output=$1 coders=$2 one_coder=$3 result=$4 counts=$5
	awk -v max_id="$coders" -v one="$one_coder" -v result="$result" -v counts="$counts" '
	BEGIN {burnouts = 0; after = 0; malformed = 0; bad_id = 0; zero = 0; missing = 0}
	{
		valid = 0
		if ($1 ~ /^[0-9]+$/ && $2 ~ /^[0-9]+$/) {
			if (NF == 6 && $3 == "has" && $4 == "taken" && $5 == "a" && $6 == "dongle") valid = 1
			if (NF == 4 && $3 == "is" && ($4 == "compiling" || $4 == "debugging" || $4 == "refactoring")) valid = 1
			if (NF == 4 && $3 == "burned" && $4 == "out") valid = 1
		}
		if (!valid) malformed = 1
		if ($2 == 0) zero = 1
		if ($2 !~ /^[0-9]+$/ || $2 < 1 || $2 > max_id) bad_id = 1
		if (burnouts > 0) after = 1
		if (valid && $3 == "has") acquisitions[$2]++
		if (valid && $3 == "is" && $4 == "compiling") {
			need = (one ? 1 : 2)
			if (acquisitions[$2] < need) missing = 1
			else acquisitions[$2] -= need
			compiles[$2]++
		}
		if (valid && $3 == "burned") {burnouts++; burned_id = $2; burned_at = $1}
	}
	END {
		if (one && acquisitions[1] != 1) missing = 1
		for (i = 1; i <= max_id; i++) print i, compiles[i] + 0 > counts
		if (zero) reason = "Coder ID 0 detected"
		else if (bad_id) reason = "Invalid coder ID"
		else if (malformed) reason = "Malformed output"
		else if (burnouts > 1) reason = "Multiple burnout messages"
		else if (after) reason = "Output after burnout"
		else if (missing) reason = "Missing dongle acquisition"
		else reason = "OK"
		print reason > result
		print burnouts >> result
		print burned_id + 0 >> result
		print burned_at + 0 >> result
	}' "$output"
}

record_failure()
{
	local label=$1 output=$2 command=$3 parity=$4 formula=$5 threshold=$6 burnout=$7
	local expected=$8 scheduler=$9 repetition=${10} exit_status=${11} coders=${12} counts=${13}
	local burned_id=${14:-0} burned_at=${15:-0} safe_name file i count

	((failed++))
	case "$label" in
		'Unexpected burnout') ((unexpected_burnout++)) ;;
		'Expected burnout did not occur') ((missing_burnout++)) ;;
		'Coder ID 0 detected'|'Invalid coder ID') ((id_failures++)) ;;
		'Malformed output') ((malformed_failures++)) ;;
		'Missing compile count') ((compile_count_failures++)) ;;
		'Missing dongle acquisition') ((dongle_failures++)) ;;
		'Output after burnout') ((output_after_burnout_failures++)) ;;
		'Multiple burnout messages') ((multiple_burnout_failures++)) ;;
		'Program timed out') ((timeout_failures++)) ;;
		'Segmentation fault') ((segfault_failures++)) ;;
		'Abort') ((abort_failures++)) ;;
		'Other signal') ((other_signal_failures++)) ;;
		'Unexpected non-zero exit') ((unexpected_nonzero++)) ;;
		'Expected rejection failed') ((expected_rejection_failed++)) ;;
	esac
	((failure_serial++))
	safe_name=$(printf '%s' "$label" | tr '[:upper:] ' '[:lower:]_' | tr -cd '[:alnum:]_-')
	file="$FAIL_DIR/run${run_id}_$(printf '%04d' "$failure_serial")_${safe_name}_${scheduler}_${coders}coders.log"
	{
		printf '%s\n' '============================================================'
		printf '[FAIL] %s\n' "$label"
		printf 'Command:\n%s\n\n' "$command"
		printf 'Coder parity: %s\n' "$parity"
		printf 'Formula: %s\n' "$formula"
		printf 'Calculated threshold: %s ms\n' "$threshold"
		printf 'Actual time_to_burnout: %s ms\n' "$burnout"
		printf 'Expected: %s\n' "$expected"
		if ((burned_id > 0)); then
			printf 'Actual: coder %s burned out\nBurnout timestamp: %s ms\n' "$burned_id" "$burned_at"
		fi
		printf 'Scheduler: %s\nRepetition: %s\nExit status: %s\n\n' "$scheduler" "$repetition" "$exit_status"
		printf 'Compile counts:\n'
		for ((i = 1; i <= coders; i++)); do
			count=$(awk -v id="$i" '$1 == id {print $2}' "$counts")
			printf 'Coder %d: %s\n' "$i" "${count:-0}"
		done
		printf '\nComplete output:\n'
		cat "$output"
		printf '%s\n' '============================================================'
	} | tee "$file"
}

run_case()
{
	local coders=$1 burnout=$2 compile_ms=$3 debug_ms=$4 refactor_ms=$5 required=$6 cooldown=$7 scheduler=$8 expected=$9
	local threshold parity multiplier formula repetitions repetition timeout_s output result counts rc reason burnouts burned_id burned_at
	local command label i count case_failed

	if ((coders % 2 == 0)); then
		parity=even; multiplier=2
	else
		parity=odd; multiplier=3
	fi
	threshold=$(((compile_ms + cooldown) * multiplier))
	formula="(time_to_compile + dongle_cooldown) * $multiplier"
	command=$(printf '%q ' "$PROGRAM" "$coders" "$burnout" "$compile_ms" "$debug_ms" "$refactor_ms" "$required" "$cooldown" "$scheduler")
	repetitions=1
	[[ "$expected" == no-burnout ]] && repetitions=$REPEAT
	timeout_s=$(calculate_timeout "$coders" "$required" "$compile_ms" "$debug_ms" "$refactor_ms" "$cooldown")
	case_failed=0
	for ((repetition = 1; repetition <= repetitions; repetition++)); do
		((total++))
		output="$tmp_dir/output.$total"
		result="$tmp_dir/result.$total"
		counts="$tmp_dir/counts.$total"
		if [[ -n "$TIMEOUT_CMD" ]]; then
			"$TIMEOUT_CMD" "${timeout_s}s" "$PROGRAM_RUN" "$coders" "$burnout" "$compile_ms" "$debug_ms" "$refactor_ms" "$required" "$cooldown" "$scheduler" >"$output" 2>&1
			rc=$?
		else
			"$PROGRAM_RUN" "$coders" "$burnout" "$compile_ms" "$debug_ms" "$refactor_ms" "$required" "$cooldown" "$scheduler" >"$output" 2>&1
			rc=$?
		fi
		validate_output "$output" "$coders" "$((coders == 1))" "$result" "$counts"
		mapfile -t validation < "$result"
		reason=${validation[0]}; burnouts=${validation[1]}; burned_id=${validation[2]}; burned_at=${validation[3]}
		label=
		if ((rc == 124)); then label='Program timed out'
		elif ((rc == 139)); then label='Segmentation fault'
		elif ((rc == 134)); then label='Abort'
		elif ((rc >= 128)); then label='Other signal'
		elif ((rc != 0)); then label='Unexpected non-zero exit'
		elif [[ "$reason" != OK ]]; then
			label=$reason
		elif [[ "$expected" == burnout && "$burnouts" -ne 1 ]]; then
			label='Expected burnout did not occur'
		elif [[ "$expected" == no-burnout && "$burnouts" -ne 0 ]]; then
			label='Unexpected burnout'
		elif [[ "$expected" == no-burnout ]]; then
			for ((i = 1; i <= coders; i++)); do
				count=$(awk -v id="$i" '$1 == id {print $2}' "$counts")
				if ((count < required)); then label='Missing compile count'; break; fi
			done
		fi
		if [[ -n "$label" ]]; then
			record_failure "$label" "$output" "$command" "$parity" "$formula" "$threshold" "$burnout" "$expected" "$scheduler" "$repetition/$repetitions" "$rc" "$coders" "$counts" "$burned_id" "$burned_at"
			case_failed=1
			break
		fi
		((passed++))
		if [[ "$VERBOSE" == 1 ]]; then cat "$output"; fi
	done
	if ((case_failed == 0)); then
		printf '[PASS] %-4s | coders=%-2d | scheduler=%-4s | expected=%s | runs=%d\n' "$parity" "$coders" "$scheduler" "$expected" "$repetitions"
	fi
}

run_matrix()
{
	local scheduler coders compile_ms cooldown threshold multiplier

	# One coder can never acquire two distinct dongles: exactly one acquisition
	# followed by burnout is required, independently of the general threshold.
	for scheduler in fifo edf; do
		run_case 1 60 20 5 5 1 0 "$scheduler" burnout
	done
	for scheduler in fifo edf; do
		for coders in 2 3 4 5 6 7 8 9 10 11; do
			compile_ms=$((20 + (coders % 4) * 10))
			cooldown=$(((coders % 3) * 10))
			if ((coders % 2 == 0)); then multiplier=2; else multiplier=3; fi
			threshold=$(((compile_ms + cooldown) * multiplier))
			# A value at or around the throughput bound is not a proof of either
			# outcome once scheduling latency and millisecond truncation apply.
			# Use an unambiguously fatal deadline and generous success margins.
			run_case "$coders" "$compile_ms" "$compile_ms" 5 5 2 "$cooldown" "$scheduler" burnout
			run_case "$coders" "$(((compile_ms + cooldown) * coders + 100))" "$compile_ms" 1 1 3 "$cooldown" "$scheduler" no-burnout
			run_case "$coders" "$(((compile_ms + cooldown) * coders + 250))" "$compile_ms" 10 10 5 "$cooldown" "$scheduler" no-burnout
		done
		# Explicit timing/cooldown/action profiles with generous safety margins.
		run_case 4 500 100 1 1 1 0 "$scheduler" no-burnout
		run_case 5 700 100 100 100 2 25 "$scheduler" no-burnout
		run_case 8 900 200 20 200 3 50 "$scheduler" no-burnout
		run_case 9 1000 50 250 250 2 100 "$scheduler" no-burnout
	done
}

run_invalid_case()
{
	local name=$1 output rc command file
	shift
	((total++))
	output="$tmp_dir/invalid.$total"
	command=$(printf '%q ' "$PROGRAM" "$@")
	if [[ -n "$TIMEOUT_CMD" ]]; then
		"$TIMEOUT_CMD" 5s "$PROGRAM_RUN" "$@" >"$output" 2>&1
		rc=$?
	else
		"$PROGRAM_RUN" "$@" >"$output" 2>&1
		rc=$?
	fi
	if ((rc > 0 && rc < 128 && rc != 124)) &&
		! grep -Eq '^[0-9]+[[:space:]]+[0-9]+[[:space:]]+(has taken a dongle|is compiling|is debugging|is refactoring|burned out)$' "$output"; then
		((passed++))
		printf '[PASS] invalid | case=%s | expected=clean-rejection\n' "$name"
		return
	fi
	((failed++))
	((expected_rejection_failed++))
	((failure_serial++))
	file="$FAIL_DIR/run${run_id}_$(printf '%04d' "$failure_serial")_expected_rejection_failed_${name}.log"
	{
		printf '%s\n' '============================================================'
		printf '[FAIL] Expected rejection failed\nCommand:\n%s\n' "$command"
		printf 'Exit status: %s\nComplete output:\n' "$rc"
		cat "$output"
		printf '%s\n' '============================================================'
	} | tee "$file"
}

run_matrix
run_invalid_case missing_arguments
run_invalid_case invalid_scheduler 4 500 20 5 5 2 0 invalid
run_invalid_case zero_coders 0 500 20 5 5 2 0 fifo
run_invalid_case nonnumeric 4 nope 20 5 5 2 0 edf

printf '\n================ TEST SUMMARY ================\n'
printf 'Total executions: %d\nPassed: %d\nFailed: %d\n' "$total" "$passed" "$failed"
printf 'Unexpected burnout failures: %d\n' "$unexpected_burnout"
printf 'Missing burnout failures: %d\n' "$missing_burnout"
printf 'Coder ID failures: %d\n' "$id_failures"
printf 'Malformed output failures: %d\n' "$malformed_failures"
printf 'Timeout failures: %d\n' "$timeout_failures"
printf 'Unexpected non-zero exit: %d\n' "$unexpected_nonzero"
printf 'Expected rejection failed: %d\n' "$expected_rejection_failed"
printf 'Segmentation faults: %d\n' "$segfault_failures"
printf 'Aborts: %d\n' "$abort_failures"
printf 'Other signals: %d\n' "$other_signal_failures"
printf 'Compile-count failures: %d\n' "$compile_count_failures"
printf 'Dongle-acquisition failures: %d\n' "$dongle_failures"
printf 'Output-after-burnout failures: %d\n' "$output_after_burnout_failures"
printf 'Multiple-burnout failures: %d\n' "$multiple_burnout_failures"
printf 'Failure logs: %s\n' "$FAIL_DIR"
printf '%s\n' '=============================================='

((failed == 0))
