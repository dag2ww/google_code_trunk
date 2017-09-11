package greg.hm.data;

import java.util.Date;

import javax.persistence.Access;
import javax.persistence.AccessType;
import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.Id;
import javax.persistence.Temporal;
import javax.persistence.TemporalType;

import org.hibernate.annotations.GenericGenerator;

@Entity
@Access(AccessType.PROPERTY)
public class MonitoringData {
	
	
	
	private Date date = new Date();

	@Id
	@GeneratedValue(strategy=GenerationType.AUTO)
	public Long getId() {
	    return this.id;
	}
	
	public void setId(long id) {
	    this.id = id;
	}
	
	@Temporal(TemporalType.TIMESTAMP)
	@Column(name = "EVENT_DATE")
	public Date getDate() {
	    return this.date;
	}
	
	@Temporal(TemporalType.TIMESTAMP)
	@Column(name = "EVENT_DATE")
	public void setDate(Date d) {
	    this.date = d;
	}
	
	private long id;
	
	private String outTemp = "0";
	private String inTemp= "0";
	private String pompWasteW= "0";
	private String pompRainW= "0";
	private String pompWarmW= "0";
	private String upTimeMain= "0";
	private String tempWarmW = "0";
	
	public String getOutTemp() {
		return outTemp;
	}
	public void setOutTemp(String outTemp) {
		this.outTemp = outTemp;
	}
	public String getInTemp() {
		return inTemp;
	}
	public void setInTemp(String inTemp) {
		this.inTemp = inTemp;
	}
	public String getPompWasteW() {
		return pompWasteW;
	}
	public void setPompWasteW(String pompWasteW) {
		this.pompWasteW = pompWasteW;
	}
	public String getPompRainW() {
		return pompRainW;
	}
	public void setPompRainW(String pompRainW) {
		this.pompRainW = pompRainW;
	}
	public String getPompWarmW() {
		return pompWarmW;
	}
	public String getTempWarmW() {
		return tempWarmW;
	}

	public void setPompWarmW(String pompWarmW) {
		this.pompWarmW = pompWarmW;
	}
	public String getUpTimeMain() {
		return upTimeMain;
	}
	public void setUpTimeMain(String upTimeMain) {
		this.upTimeMain = upTimeMain;
	}
	public void setTempWarmW(String tempWarmW) {
		this.tempWarmW = tempWarmW;
	}
	
	

}
