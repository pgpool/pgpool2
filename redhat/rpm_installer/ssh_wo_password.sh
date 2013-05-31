#!/bin/sh

USERNAME=$1
THIS_HOST=$3
DEST_HOST=$3

SSH_DIR=/home/$USERNAME/.ssh

# config SSH in remote host
echo "    - regist $DEST_HOST's public key to $THIS_HOST"

su - $USERNAME -c "mkdir $SSH_DIR > /dev/null 2>&1"

PUB_KEY=`ssh $USERNAME@$DEST_HOST -T << EOF2
rm $SSH_DIR/id_rsa*
ssh-keygen -q -t rsa -P '' -f $SSH_DIR/id_rsa << EOF

EOF

chmod 700 $SSH_DIR
touch $SSH_DIR/authorized_keys
chmod 600 $SSH_DIR/authorized_keys

cat $SSH_DIR/id_rsa.pub
EOF2
`
if [ $? -ne 0 ]; then exit 1; fi

su - $USERNAME -c "echo $PUB_KEY >> $SSH_DIR/authorized_keys"
if [ $? -ne 0 ]; then exit 1; fi

# config SSH in local host
echo "    - Regist $THIS_HOST's public key to $DEST_HOST"
rm $SSH_DIR/id_rsa*
su - $USERNAME -c "ssh-keygen -q -t rsa -P '' -f $SSH_DIR/id_rsa << EOF

EOF"
if [ $? -ne 0 ]; then exit 1; fi

ssh-copy-id -i $SSH_DIR/id_rsa.pub $USERNAME@$DEST_HOST > /dev/null 2>&1
if [ $? -ne 0 ]; then exit 1; fi

# arrange permissions
chmod 700 $SSH_DIR
chmod 600 $SSH_DIR/authorized_keys

exit 0
