package org.lanma.lassandroid;

/**
 * Created by mac on 2016/2/17.
 */
public class LassInfo {
    public String sDevId;
    public String sDate;
    public String sTime;
    public String sPM25;
    public String sTemp;
    public String sHumi;
    public String sLat;
    public String sLng;
    public LassInfo()
    {
        clearData();
    }
    public void clearData()
    {
        sDevId="";
        sDate="";
        sTime="";
        sPM25="";
        sTemp="";
        sHumi="";
        sLat="";
        sLng="";
    }
}
