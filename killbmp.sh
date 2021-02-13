#!/bin/bash

for f in $(find|grep '\.bmp$'); do
	rm $f
done
