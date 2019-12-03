#!/usr/bin/env bash
apt-get update && apt-get install -y postgresql unixodbc unixodbc-dev odbc-postgresql
odbcinst -i -d -f /usr/share/psqlodbc/odbcinst.ini.template
odbcinst -i -s -l -n edat-pg -f /usr/share/doc/odbc-postgresql/examples/odbc.ini.template
service postgresql start
sudo -u postgres psql -c "CREATE USER alumnodb WITH PASSWORD 'alumnodb' CREATEDB;"

#Para arrancar, usar su postgres
