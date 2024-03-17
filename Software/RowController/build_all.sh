# Build all 13 Row Controllers

for addr in {1..13}; do
    echo "Building RowController @ Addr $addr"

    LOG_FILE="artifacts/buildlogs/$addr.txt"
    bash ./build_one.sh $addr >$LOG_FILE 2>&1
done
