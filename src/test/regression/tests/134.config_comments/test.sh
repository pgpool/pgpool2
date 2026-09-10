#!/usr/bin/env bash
# Test configuration files whose last line has no newline.

set -e -o pipefail

TESTDIR=testdir
PSQL=$PGBIN/psql

rm -fr "$TESTDIR"
mkdir "$TESTDIR"
cd "$TESTDIR"

$PGPOOL_SETUP -m s -n 1
source ./bashrc.ports
export PGPORT=$PGPOOL_PORT
cp etc/pgpool.conf pgpool.conf.base

cleanup()
{
    # The stop command also parses the configuration file.
    cp pgpool.conf.base etc/pgpool.conf
    ./shutdownall
}
trap cleanup EXIT

check_setting()
{
    local name=$1
    local expected=$2
    local result
    local attempt

    for attempt in {1..20}
    do
        if result=$(timeout 5 "$PSQL" -XAt -v ON_ERROR_STOP=1 \
            -c "SHOW POOL_STATUS" test 2>/dev/null |
            awk -F '|' -v name="$name" '$1 == name {print $2}') &&
            [ "$result" = "$expected" ]; then
            echo "$name = $expected: ok"
            return 0
        fi
        sleep 1
    done

    echo "Expected $name = $expected, got: $result"
    cat log/pgpool.log
    return 1
}

echo "Standalone comment at EOF on startup"
printf '\nclient_idle_limit = 60\n# Regexp are accepted' >> etc/pgpool.conf
./startall
check_setting client_idle_limit 60

echo "Inline comment at EOF on reload"
cp pgpool.conf.base etc/pgpool.conf
printf '\nclient_idle_limit = 61 # comment' >> etc/pgpool.conf
./pgpool_reload
check_setting client_idle_limit 61

echo "Empty comment at EOF on reload"
cp pgpool.conf.base etc/pgpool.conf
printf '\nclient_idle_limit = 62\n#' >> etc/pgpool.conf
./pgpool_reload
check_setting client_idle_limit 62

echo "Assignment at EOF without a comment"
cp pgpool.conf.base etc/pgpool.conf
printf '\nclient_idle_limit = 63' >> etc/pgpool.conf
./pgpool_reload
check_setting client_idle_limit 63

echo "Quoted hash followed by a comment at EOF"
cp pgpool.conf.base etc/pgpool.conf
printf "\nlog_line_prefix = 'config # prefix' # comment" >> etc/pgpool.conf
./pgpool_reload
check_setting log_line_prefix 'config # prefix'

echo "Comment at EOF in an included file"
printf 'client_idle_limit = 64 # comment' > etc/included.conf
cp pgpool.conf.base etc/pgpool.conf
cat >> etc/pgpool.conf <<'EOF'

include = 'included.conf'
client_idle_limit_in_recovery = 65 # comment followed by a newline
EOF
./pgpool_reload
check_setting client_idle_limit 64
check_setting client_idle_limit_in_recovery 65
