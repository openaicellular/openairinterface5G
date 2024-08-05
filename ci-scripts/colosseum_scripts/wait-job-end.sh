#!/bin/bash
set -xeu

AWX_API_URL=https://open6g-awx.colosseum.net
AWX_MAX_API_CHECKS=120

COL_USER=$1
COL_PASS=$2

AWX_API_JOB_PATH=$(jq -r '.url' launch.json)

# wait for job to complete
for try in $(seq ${AWX_MAX_API_CHECKS}); do
  set +x
  curl -s -f -k -u ${COL_USER}:${COL_PASS} -X GET ${AWX_API_URL}${AWX_API_JOB_PATH} > status.json
  set -x

  FINISHED=$(jq -r '.finished' status.json)
  [ "${FINISHED}" = "null" ] || break

  sleep 60
done

[ $try != $AWX_MAX_API_CHECKS ] || echo "WARNING: stopped retrying after $AWX_MAX_API_CHECKS times; timed out?"

echo "AWX job completed $FINISHED"
