import java.io.PrintWriter;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.Statement;
import java.util.Properties;

import org.postgresql.Driver;

public class Sample {
    public Sample() {
    }

    public static void main(String[] args) throws Exception {
        ResultSet rs;
        Properties props = new Properties();
        props.setProperty("user", "t-ishii");
        props.setProperty("password", "");
        props.setProperty("protocolVersion", "3");
        DriverManager.setLogWriter(new PrintWriter(System.out));
        Driver.setLogLevel(2);
        Connection conn = DriverManager.getConnection(
                "jdbc:postgresql://localhost:11000/test", props);
        conn.setAutoCommit(true);
        Statement st = conn.createStatement();
        st.setFetchSize(100);
	// Does not hit cache
        rs = st.executeQuery("SELECT 1");
        while (rs.next()) {
            System.out.println(rs.getString(1));
        }
        rs.close();

	// Does hit cache
        rs = st.executeQuery("SELECT 1");
        while (rs.next()) {
            System.out.println(rs.getString(1));
        }
        rs.close();

	// To call do_query()
        rs = st.executeQuery("SELECT * FROM t1");
        while (rs.next()) {
            System.out.println(rs.getString(1));
        }
        rs.close();

	conn.close();
    }
}
