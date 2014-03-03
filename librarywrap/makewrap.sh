#
#set -x
set -e
for fname in `sort -u /tmp/xx.logfile  | sed -e "s/].*//" -e "s/.*\[//"`
do
echo "WRAP:" $fname
grep -w $fname ftdiwrap.h | grep -v dlsym | sed -e "s/^/\/\//"
done
# -e "s/^/WRAP: /"
