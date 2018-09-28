#!/usr/bin/env bash

# Create root cert
openssl req -new -x509 -nodes -out root.crt -keyout root.key -subj /CN=MyRootCA
# PostgreSQL/Pgpool cert
openssl req -new -out server.req -keyout server.key -nodes -subj "/CN=postgresql"
openssl x509 -req -in server.req -CAkey root.key -CA root.crt -days 365 -CAcreateserial -out server.crt
# Frontend Cert
openssl req -new -out postgresql.req -keyout frontend.key -nodes -subj "/CN=$USER"
openssl x509 -req -in postgresql.req -CAkey root.key -CA root.crt -days 365 -CAcreateserial -out frontend.crt
