#! /bin/msh

echo "Content-Type: text/plain"
echo

CONFIG_FILENAME="/tmp/leanxsugus-config"

echo "$QUERY_STRING" | tr '&' '\n' > "$CONFIG_FILENAME~"
mv -f "$CONFIG_FILENAME~" "$CONFIG_FILENAME"
killall -1 leanxsugus
echo "Configration saved successfully!"
