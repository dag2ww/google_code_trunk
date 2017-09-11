package greg.hm.hibernate;

import java.util.List;

import greg.hm.MonitoringInfo;
import greg.hm.data.MonitoringData;

import org.hibernate.Session;
import org.hibernate.SessionFactory;
import org.hibernate.cfg.Configuration;

public class HibernateManager {

	private static HibernateManager theInstance;

	private HibernateManager() {

	}

	public static void main(String[] args) {
		for(MonitoringData row : getInstance().getAllMonitoringEntities()){
			System.out.println("czas:"+row.getDate());
		}

	}

	public static HibernateManager getInstance() {
		if (theInstance == null) {
			theInstance = new HibernateManager();
			theInstance.init();
		}
		return theInstance;
	}

	SessionFactory sessionFactory;
	private long lastSaved = 0;

	private void init() {
		// A SessionFactory is set up once for an application
		sessionFactory = new Configuration().configure() // configures settings
															// from
															// hibernate.cfg.xml
				.buildSessionFactory();
	}

	public void test() {
		Session session = sessionFactory.openSession();
		session.beginTransaction();
		session.save(new MonitoringData());
		// session.save( new MonitoringData() );
		session.getTransaction().commit();

		// ///////
		// session.beginTransaction();
		// List result = session.createQuery( "from Event" ).list();
		// for ( Event event : (List<Event>) result ) {
		// System.out.println( "Event (" + event.getDate() + ") : " +
		// event.getTitle() );
		// }
		// session.getTransaction().commit();

		session.close();
	}

	public void saveToDB(MonitoringData data) {

		if (data.getDate().getTime()-lastSaved > (1000 * 60)) {
			lastSaved = data.getDate().getTime();
			Session session = sessionFactory.openSession();
			session.beginTransaction();
			session.save(data);
			session.getTransaction().commit();
			session.close();
		}
	}

	@SuppressWarnings("unchecked")
	public List<MonitoringData> getAllMonitoringEntities() {
		Session session = sessionFactory.openSession();
		// session.beginTransaction();
		List<MonitoringData> result = session
				.createQuery("From MonitoringData").list();
		// session.getTransaction().commit();
		session.close();
		return result;
	}

}
