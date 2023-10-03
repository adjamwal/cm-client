#!/bin/bash
#
# Usage:
#
# create_bs_json.sh <business_id> <event_url> <identify_url> <installer_key> <filename.json>
#
# Example:
# % ./create_bs_json.sh "40f60118-a793-5f47-93bf-2f93f4140404" "https://identify.qa.uc.amp.cisco.com/event" \
# "https://identify.qa.uc.amp.cisco.com/identify" "58c0be0d-4ab4-40ae-9ff5-0d1a4f19a57a" bs.json

if [ $# -ne 5 ]; then
  echo "Usage: create_bs_json.sh <business_id> <event_url> <identify_url> <installer_key> <filename.json>"
  exit 1
fi

BS_BUSINESS_ID=$1
BS_EVENT_URL=$2
BS_IDENTIFY_URL=$3
BS_INSTALLER_KEY=$4

cat << EOF > "${5}"
{
  "business_id":"${BS_BUSINESS_ID}",
  "event_url":"${BS_EVENT_URL}",
  "identify_url":"${BS_IDENTIFY_URL}",
  "installer_key":"${BS_INSTALLER_KEY}"
}
EOF

exit 0
