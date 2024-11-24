#!/usr/bin/env bash

[ -e "index.txt" ] && rm "index.txt"
touch index.txt
echo '1000' > serial
echo 'unique_subject = yes/no' > index.txt.attr
echo '1000' >  crlnumber
if [ -d "certrecord" ]; then rm -Rf certrecord; fi
mkdir certrecord
if [ -d "newcerts" ]; then rm -Rf newcerts; fi
mkdir newcerts

cat > crl_openssl.conf <<EOF

[ ca ]
default_ca = CA_default

[ CA_default ]
dir               = .
database          = index.txt
serial            = serial
certs             = newcerts
new_certs_dir     = certrecord

default_md        = sha256
crlnumber         = crlnumber
default_crl_days  = 365

name_opt          = ca_default
cert_opt          = ca_default
default_days      = 375
preserve          = no
policy            = policy_loose

# The root key and root certificate.
private_key       = root.key
certificate       = root.crt

[ policy_loose ]
# Allow the intermediate CA to sign a more diverse range of certificates.
# See the POLICY FORMAT section of the `ca` man page.
countryName             = optional
stateOrProvinceName     = optional
localityName            = optional
organizationName        = optional
organizationalUnitName  = optional
commonName              = supplied
emailAddress            = optional

[req]
distinguished_name  = req_distinguished_name
default_bits        = 2048

[req_distinguished_name]

EOF

# Print OpenSSL version
openssl version

# OpenSSL config file dir
dir=`openssl version -d|awk '{print $2}'|sed 's/"//g'`

# Create root cert
openssl req -new -nodes -text -out root.csr -keyout root.key -subj "/CN=MyrootCA"
chmod og-rwx root.key
openssl x509 -req -in root.csr -text -days 3650 -extfile $dir/openssl.cnf -extensions v3_ca -signkey root.key -out root.crt

# PostgreSQL/Pgpool cert
openssl req -new -nodes -text -out server.csr -keyout server.key -subj "/CN=postgresql"
chmod og-rwx server.key
openssl x509 -req -in server.csr -text -days 365 -CA root.crt -CAkey root.key -CAcreateserial -out server.crt

# Frontend Cert
openssl req -new -nodes -text -out frontend.csr -keyout frontend.key -subj "/CN=$USER"
chmod og-rwx frontend.key
openssl x509 -req -in frontend.csr -text -days 365 -CA root.crt -CAkey root.key -CAcreateserial -out frontend.crt

# Generate clean CRL (No revocation so far)
openssl ca -config crl_openssl.conf -gencrl -out server.crl -cert root.crt -keyfile root.key

# Revoke Frontend Cert
openssl ca -config crl_openssl.conf -revoke frontend.crt -keyfile root.key -cert root.crt -out root.crl

# Generate CRL after revocation
openssl ca -config crl_openssl.conf -gencrl -out server_revoked.crl -cert root.crt -keyfile root.key
