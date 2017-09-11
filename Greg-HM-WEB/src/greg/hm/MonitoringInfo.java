package greg.hm;

import greg.hm.data.DBConnector;
import greg.hm.data.MonitoringData;
import greg.hm.hibernate.HibernateManager;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

import javax.servlet.ServletContext;
import javax.servlet.http.HttpServletRequest;
import javax.ws.rs.FormParam;
import javax.ws.rs.GET;
import javax.ws.rs.POST;
import javax.ws.rs.Path;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;
import javax.ws.rs.core.MediaType;

import com.google.visualization.datasource.datatable.ColumnDescription;
import com.google.visualization.datasource.datatable.DataTable;
import com.google.visualization.datasource.datatable.value.ValueType;
import com.google.visualization.datasource.render.JsonRenderer;
import com.ibm.icu.util.GregorianCalendar;
import com.ibm.icu.util.TimeZone;

/**
 * Defines MonitoringInfo resource
 * 
 * @author Grzegorz
 * 
 */
@Path("/status")
public class MonitoringInfo {
	
	public static void main(String[] arg){
		MonitoringInfo mi = new MonitoringInfo();
		System.out.print(mi.generateTemperaturesDataTable());
	}

	// A Servlet- based implementation MUST support injection of the following
	// Servlet-defined types: ServletConfig, ServletContext, HttpServletRequest
	// and HttpServletResponse.

	@javax.ws.rs.core.Context
	HttpServletRequest myRequest;

	@javax.ws.rs.core.Context
	ServletContext context;

	// used to get data from web service
	@GET
	@Produces(MediaType.TEXT_HTML)
	public String getMainStatusInfo() throws IOException {
		InputStream inStream = context
				.getResourceAsStream("Resources/main_page.html");
		InputStreamReader reader = new InputStreamReader(inStream);
		BufferedReader breader = new BufferedReader(reader);
		StringBuilder result = new StringBuilder();
		String line;

		// File f = new File("data.txt");
		// FileInputStream fin = null;
		// ObjectInputStream oin = null;
		//
		// HashMap<String,String> data = new HashMap<String,String>();
		// try{
		// fin = new FileInputStream(f);
		// oin = new ObjectInputStream(fin);
		// data = (HashMap<String,String>)oin.readObject();
		// } catch (Exception e){
		// e.printStackTrace();
		// }
		// finally{
		// oin.close();
		// fin.close();
		// }

		MonitoringData data = (MonitoringData) context
				.getAttribute("monitoring.data");
		if (data == null) {
			data = new MonitoringData();
			context.setAttribute("monitoring.data", data);
		}

		while ((line = breader.readLine()) != null) {
			line = line.replace("${outdoor.temperature}",
					"" + data.getOutTemp());
			line = line.replace("${indoor.temperature}", "" + data.getInTemp());
			line = line.replace("${pomp.waste.water}",
					"" + data.getPompWasteW());
			line = line.replace("${pomp.rain.water}", "" + data.getPompRainW());
			line = line.replace("${pomp.warm.water}", "" + data.getPompWarmW());
			line = line.replace("${temp.warm.water}", "" + data.getTempWarmW());
			line = line.replace(
					"${up.time.main.station}",
					"" + data.getUpTimeMain() + " | UC: "
							+ context.getAttribute("update.counter"));
			result.append(line + "\r\n");
		}
		return result.toString();
	}

	// used to get data from web service
	@GET
	@Produces(MediaType.TEXT_PLAIN)
	public String getMainStatusInfoPlainText() {
		return "Works plain text";
	}

	// used to put data into web service
	// curl --data "birthyear=1905&press=%20OK%20"
	// http://www.example.com/when.cgi
	// curl --data-urlencode "name=I am Daniel" http://www.example.com
	// curl --data-urlencode "outdoor.temperature=27"
	// http://192.168.1.104:2000/greg-hm/rest/update
	// curl --data
	// "outdoor.temperature=33&indoor.temperature=11&pomp.waste.water=7&pomp.rain.water=5&pomp.warm.water=8&up.time.main.station=9"
	// http://localhost:8080/greg-hm/rest/status/update/ @POST
	// WiFly: set com remote
	// POST$/greg-hm/rest/status/update?outdoor.temperature=33&indoor.temperature=1
	// WiFly: set com remote GET$/greg-hm/rest/status/ctrl - get the controll
	// string
	// upon boot: MAIN_STATION-BOOT
	@POST
	@Path("/update")
	@Produces(MediaType.TEXT_PLAIN)
	public String statusUpdate(
			@FormParam(value = "outdoor.temperature") String outTempF,
			@FormParam(value = "indoor.temperature") String inTempF,
			@FormParam(value = "pomp.waste.water") String pompWasteWF,
			@FormParam(value = "pomp.rain.water") String pompRainWF,
			@FormParam(value = "pomp.warm.water") String pompWarmWF,
			@FormParam(value = "temp.warm.water") String tempWarmWF,
			@FormParam(value = "up.time.main.station") String upTimeMainF,
			@QueryParam("outdoor.temperature") String outTemp,
			@QueryParam("indoor.temperature") String inTemp,
			@QueryParam("pomp.waste.water") String pompWasteW,
			@QueryParam("pomp.rain.water") String pompRainW,
			@QueryParam("pomp.warm.water") String pompWarmW,
			@QueryParam("temp.warm.water") String tempWarmW,
			@QueryParam("up.time.main.station") String upTimeMain) {

		System.err.println("in: " + inTemp);
		System.err.println("out: " + outTemp);

		context.setAttribute("update.counter", context
				.getAttribute("update.counter") == null ? 1
				: ((Integer) context.getAttribute("update.counter")) + 1);
		// do update of currently displayed values
		// save values e.g. in servlet session
		MonitoringData data = handleMonitoringDataUpdate(outTemp, inTemp,
				pompWasteW, pompRainW, pompWarmW, tempWarmW, upTimeMain);
		context.setAttribute("monitoring.data", data);

		// put into DB
		HibernateManager.getInstance().saveToDB(data);

		return "ControllStringData";
	}

	@GET
	@Path("/pomps_chart_data")
	@Produces(MediaType.APPLICATION_JSON)
	public String getPompsChartData() {
		return renderDataTableToJSON(generatePompsDataTable()).toString();

	}

	@GET
	@Path("/temps_chart_data")
	@Produces(MediaType.APPLICATION_JSON)
	public String getTempsChartData() {

		return renderDataTableToJSON(generateTemperaturesDataTable())
				.toString();

	}

	private MonitoringData handleMonitoringDataUpdate(String outTemp,
			String inTemp, String pompWasteW, String pompRainW,
			String pompWarmW, String tempWarmW, String upTimeMain) {

		MonitoringData data = new MonitoringData();

		if (outTemp != null && !outTemp.isEmpty()) {
			data.setOutTemp(outTemp);
		}
		if (inTemp != null && !inTemp.isEmpty()) {
			data.setInTemp(inTemp);
		}
		if (pompWasteW != null && !pompWasteW.isEmpty()) {
			data.setPompWasteW(pompWasteW);
		}
		if (pompRainW != null && !pompRainW.isEmpty()) {
			data.setPompRainW(pompRainW);
		}
		if (pompWarmW != null && !pompWarmW.isEmpty()) {
			data.setPompWarmW(pompWarmW);
		}
		if (tempWarmW != null && !tempWarmW.isEmpty()) {
			data.setTempWarmW(tempWarmW);
		}
		if (upTimeMain != null && !upTimeMain.isEmpty()) {
			data.setUpTimeMain(upTimeMain);
		}
		return data;
	}

	// HashMap<String, String> data = new HashMap<String, String>();
	// data.put("outdoortemperature", ""+outTemp);
	// data.put("indoortemperature", ""+inTemp);
	// File f = new File("data.txt");
	// FileOutputStream fout = null;
	// ObjectOutputStream oout = null;
	// try{
	// fout = new FileOutputStream(f);
	// oout = new ObjectOutputStream(fout);
	// oout.writeObject(data);} catch (Exception exc){
	// exc.printStackTrace();
	// } finally {
	//
	// try {
	// oout.close();
	// fout.close();
	// } catch (IOException ex) {
	// // Auto-generated catch block
	// ex.printStackTrace();
	// }
	// }

	public DataTable generatePompsDataTable() {

		// Create a data table,
		DataTable data = new DataTable();
		ArrayList<ColumnDescription> cd = new ArrayList<ColumnDescription>();

		cd.add(new ColumnDescription("czas", ValueType.DATETIME, "Czas"));
		cd.add(new ColumnDescription("oczyszczalnia", ValueType.NUMBER,
				"Oczyszczalnia"));
		cd.add(new ColumnDescription("title1", ValueType.TEXT, "Title1"));
		cd.add(new ColumnDescription("text1", ValueType.TEXT, "Text1"));
		cd.add(new ColumnDescription("deszczowka", ValueType.NUMBER,
				"Deszczówka"));
		cd.add(new ColumnDescription("cyrkulacja", ValueType.NUMBER,
				"Cyrkulacja"));
		cd.add(new ColumnDescription("title2", ValueType.TEXT, "Title2"));
		cd.add(new ColumnDescription("text2", ValueType.TEXT, "Title2"));

		data.addColumns(cd);

		// Fill the data table.
		try {
			List<MonitoringData> last100Entities = HibernateManager
					.getInstance().getAllMonitoringEntities();
			for (MonitoringData dataItem : last100Entities) {
				GregorianCalendar calendar = new GregorianCalendar(
						TimeZone.getTimeZone("GMT"));
				calendar.setTime(dataItem.getDate());
				// calendar.set(2008, 1 ,1, 22,22,22);
				data.addRowFromValues(calendar,
						(dataItem.getPompWasteW()==""?0:1), null, null,
						(dataItem.getPompRainW()==""?0:1),
						(dataItem.getPompWarmW()==""?0:1), null, null);
			}
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("Invalid type!");
		}

		return data;
	}

	public DataTable generateTemperaturesDataTable() {

		// Create a data table,
		DataTable data = new DataTable();
		ArrayList<ColumnDescription> cd = new ArrayList<ColumnDescription>();

		cd.add(new ColumnDescription("czas", ValueType.DATETIME, "Czas"));
		cd.add(new ColumnDescription("nadworze", ValueType.NUMBER, "Na Dworze"));
		cd.add(new ColumnDescription("title1", ValueType.TEXT, "Title1"));
		cd.add(new ColumnDescription("text1", ValueType.TEXT, "Text1"));
		cd.add(new ColumnDescription("wdomu", ValueType.NUMBER, "W Domu"));
		cd.add(new ColumnDescription("cieplawoda", ValueType.NUMBER,
				"Ciep³a Woda"));
		cd.add(new ColumnDescription("title2", ValueType.TEXT, "Title2"));
		cd.add(new ColumnDescription("text2", ValueType.TEXT, "Title2"));

		data.addColumns(cd);

		// Fill the data table.
		try {
			List<MonitoringData> last100Entities = HibernateManager
					.getInstance().getAllMonitoringEntities();
			for (MonitoringData dataItem : last100Entities) {
				
			GregorianCalendar calendar = new GregorianCalendar(
					TimeZone.getTimeZone("GMT"));
			calendar.setTime(dataItem.getDate());
			data.addRowFromValues(calendar, Double.parseDouble(((dataItem.getOutTemp().equals("-----"))?"0":dataItem.getOutTemp())), null, null, Double.parseDouble(dataItem.getInTemp()), Double.parseDouble(dataItem.getTempWarmW()), null,
					null);
			}
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("Invalid type!");
		}

		return data;
	}

	CharSequence renderDataTableToJSON(DataTable dataTable) {
		return JsonRenderer.renderDataTable(dataTable, true, true, false);
	}

}
