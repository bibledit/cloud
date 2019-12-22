#!/bin/sh

echo Start two Firefox browsers
PORT1=4444
PORT2=5555

geckodriver --port $PORT1 &
PID1=$!
geckodriver --port $PORT2 &
PID2=$!

sleep 1

curl -d '{"capabilities": {"alwaysMatch": {"acceptInsecureCerts": true}}}' http://localhost:$PORT1/session | tee out.txt
# This outputs something like this:
# {"value": {"sessionId":"e46f5e73-7dd2-7b42-9fd5-0a6483933097","capabilities":{"acceptInsecureCerts":true,"browserName":"firefox","browserVersion":"71.0","capabilities":{"desiredCapabilities":{"acceptInsecureCerts":true}},"moz:accessibilityChecks":false,"moz:buildID":"20191202093317","moz:headless":false,"moz:processID":32663,"moz:profile":"/var/folders/z9/lc690hv133qfzcy5cz65jcp00000gn/T/rust_mozprofile.KUJw4HCgDqHX","moz:shutdownTimeout":60000,"moz:useNonSpecCompliantPointerOrigin":false,"moz:webdriverClick":true,"pageLoadStrategy":"normal","platformName":"mac","platformVersion":"19.2.0","rotatable":false,"setWindowRect":true,"strictFileInteractability":false,"timeouts":{"implicit":0,"pageLoad":300000,"script":30000},"unhandledPromptBehavior":"dismiss and notify"}}}
SESSION1=`cat out.txt | cut -d \" -f6`
echo

curl -d '{"capabilities": {"alwaysMatch": {"acceptInsecureCerts": true}}}' http://localhost:$PORT2/session | tee out.txt
SESSION2=`cat out.txt | cut -d \" -f6`
rm out.txt
echo

sleep 1

curl -d '{"url": "https://bibledit.org:8081/editone/index"}' http://localhost:$PORT1/session/$SESSION1/url
curl -d '{"url": "https://bibledit.org:8081/editone/index"}' http://localhost:$PORT2/session/$SESSION2/url
# curl http://localhost:$PORT1/session/$SESSION1/url

sleep 3
echo
echo Session 1 is $SESSION1 on port $PORT1
echo Session 2 is $SESSION2 on port $PORT2
echo Press any key to close the sessions and browsers
read -n 1
curl -X DELETE http://localhost:$PORT1/session/$SESSION1
curl -X DELETE http://localhost:$PORT2/session/$SESSION2
sleep 3
kill -9 $PID1
kill -9 $PID2
killall geckodriver
echo
echo

