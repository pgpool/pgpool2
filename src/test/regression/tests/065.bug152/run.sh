#JDBC_DRIVER=/usr/local/pgsql/share/postgresql-9.4-1204.jdbc41.jar
JDBC_DRIVER=/usr/local/pgsql/share/postgresql-9.2-1003.jdbc4.jar
export CLASSPATH=.:$JDBC_DRIVER
javac Main.java
java Main
