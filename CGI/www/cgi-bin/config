#! /bin/msh

echo "Content-Type: text/plain"
echo

CONFIG_FILENAME="/tmp/leanxsugus-config"
KEY=`echo "$QUERY_STRING" | cut -s -d "=" -f 1`
VALUE=`echo "$QUERY_STRING" | cut -s -d "=" -f 2`
HAD_KEY=0

[ "$KEY" == "" ] && echo "Error: No key specified!" && exit 1
[ -f "$CONFIG_FILENAME" ] || touch "$CONFIG_FILENAME"
[ -f "$CONFIG_FILENAME~" ] || echo -n > "$CONFIG_FILENAME~"

cat "$CONFIG_FILENAME" | while read i; do
	if [ $HAD_KEY -gt 0 ]; then
		echo "$i" >> "$CONFIG_FILENAME~"
	else
		KEY_CURR=`echo "$i" | cut -s -d "=" -f 1`
		if [ "$KEY_CURR" == "$KEY" ]; then
			if [ "$VALUE" != "" ]; then
				echo "$KEY=$VALUE" >> "$CONFIG_FILENAME~"
			fi
			HAD_KEY=1
		else
			echo "$i" >> "$CONFIG_FILENAME~"
		fi
	fi
done

[ $HAD_KEY -gt 0 ] || echo "$KEY=$VALUE" >> "$CONFIG_FILENAME~"

mv -f "$CONFIG_FILENAME~" "$CONFIG_FILENAME"
killall -1 leanxsugus
echo "Configration saved successfully!"
