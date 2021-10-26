#!/bin/bash

set -e

kernel_name=$1
build_dir=./build_dir.hw.xilinx_aws-vu9p-f1_shell-v04261818_201920_2

xclbin=$(find $build_dir -iname "$kernel_name.xclbin")
create_vitis_afi="$VITIS_DIR/tools/create_vitis_afi.sh"
BUCKET_NAME="my-bucket-fpga-jero"
FOLDER="$kernel_name"_hls
LOGS="$FOLDER/logs"
touch FILES_GO_HERE.txt
touch LOGS_GO_HERE.txt
aws s3 mb s3://$BUCKET_NAME/$FOLDER
aws s3 cp FILES_GO_HERE.txt s3://$BUCKET_NAME/$FOLDER/
aws s3 mb s3://$BUCKET_NAME/$LOGS
aws s3 cp LOGS_GO_HERE.txt s3://$BUCKET_NAME/$LOGS/
$create_vitis_afi -xclbin=$xclbin -o=$kernel_name -s3_bucket=$BUCKET_NAME -s3_dcp_key=$FOLDER -s3_logs_key=$LOGS

