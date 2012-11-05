#!/bin/sh

TERMINAL=/Applications/Utilities/Terminal.app

if [ "${ACTION}" = "run" ]; then
    echo "will run debugger"
    open -a ${TERMINAL} ${PROJECT_DIR}/dbg_client.rb
else
    echo "action: ${ACTINO}"
fi
