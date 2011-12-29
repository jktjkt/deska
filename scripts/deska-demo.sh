#!/bin/bash

function die() {
    echo $1
    exit 1
}

/bin/ls -al deska-*.rpm > /dev/null 2> /dev/null \
    || die "Please run the deska-demo.sh from a directory which contains the RPM packages."

echo "Installing packages..."

# Install the packages
yum -y localinstall *.rpm || die "Error when installing packages"

yum install -y yum-conf-epel
yum install -y GitPython

echo "Starting PostgreSQL..."
service postgresql-9.0 initdb || die "Cannot initialize the PostgreSQL server"
service postgresql-9.0 start || die "Cannot start PostgreSQL server"

echo "Creating user franta..."
useradd franta || die "Cannot add user franta"
echo -e "franta\nfranta\n" | passwd franta --stdin || die "Cannot change franta's password"
echo -e "[DBConnection]\nServer=deska-server-wrapper\n" > ~franta/deska.ini

# FIXME: produce the deska-server-wrapper with a proper git integration

echo "Creating the PostgreSQL database"

su postgres -c 'psql -q -U postgres -c "DROP DATABASE IF EXISTS d_fzu;"' || die "Drop database"
su postgres -c 'psql -q -U postgres -c "CREATE ROLE deska_admin;"' || die "Create role deska_admin"
su postgres -c 'psql -q -U postgres -c "CREATE ROLE deska_user;"' || die "Create role deska_user"
su postgres -c 'psql -q -U postgres -c "CREATE USER franta;"' || die "Create user franta"
su postgres -c 'psql -q -U postgres -c "GRANT deska_admin TO franta;"' || die "Cannot grant deska_admin"
su postgres -c 'psql -q -U postgres -c "GRANT deska_user TO franta;"' || die "Cannot grant deska_user"
su postgres -c 'psql -q -U postgres -c "CREATE DATABASE d_fzu OWNER deska_admin;"' || die "Create database"

su postgres -c 'psql -q -U postgres -d d_fzu -v ON_ERROR_STOP=1 -f /usr/share/deska/install-scripts/install/pgpython.sql' || die "Installing pgpython"

echo "Deploying Deska into the database"
cd /usr/share/deska/install-scripts/install

mkdir -p /var/lib/deska
chown postgres:postgres /var/lib/deska

su postgres -c "DESKA_SCHEME=fzu ./deska-deploy-database.sh -U postgres -d d_fzu -t /var/lib/deska/d_fzu" \
    || die "Cannot deploy the database scheme"

echo "All done. Please login as user 'franta' (pw 'franta') and run 'deska-cli' to evaluate the Deska system."
