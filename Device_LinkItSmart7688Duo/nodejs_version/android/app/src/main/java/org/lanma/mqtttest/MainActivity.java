package org.lanma.mqtttest;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;

public class MainActivity extends AppCompatActivity implements MqttCallback {
    private static String clientId = "TurtleCarAndroid";
    private static MqttClient mqttClient;
    //public static final String TOPIC = "TurtleCar";
    public static final String TOPIC_STATUS = "LASS/Test/PM25";
    public static final int QOS = 0;
    public static final int TIMEOUT = 3;

    private LassInfo m_li;
    TextView m_tvDevId;
    TextView m_tvPM25;
    TextView m_tvTemp;
    TextView m_tvHumi;
    TextView m_tvDateTime;
    EditText m_etFilter;
    private void initUI()
    {
        m_tvDevId = (TextView)this.findViewById(R.id.tvDevId);
        m_tvPM25 = (TextView)this.findViewById(R.id.tvPM25);
        m_tvTemp = (TextView)this.findViewById(R.id.tvTemp);
        m_tvHumi = (TextView)this.findViewById(R.id.tvHumi);
        m_tvDateTime = (TextView)this.findViewById(R.id.tvDateTime);
        m_etFilter = (EditText)this.findViewById(R.id.etFilter);
    }
    private void processConnect(String brokerIp, String brokerPort) {
        String broker = "tcp://" + brokerIp + ":" + brokerPort;

        try {
            clientId = clientId + System.currentTimeMillis();

            MqttConnectOptions mqttConnectOptions = new MqttConnectOptions();
            mqttConnectOptions.setCleanSession(true);
            mqttConnectOptions.setConnectionTimeout(TIMEOUT);

            mqttClient = new MqttClient(broker, clientId, new MemoryPersistence());
            mqttClient.connect(mqttConnectOptions);
            mqttClient.subscribe(TOPIC_STATUS);
            mqttClient.setCallback(this);

            Toast.makeText(this, "MQTT Broker Connected", Toast.LENGTH_LONG).show();
        }
        catch (MqttException me) {
            Toast.makeText(this, "MQTT Broker Connect failed", Toast.LENGTH_LONG).show();
        }
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        initUI();
        m_li = new LassInfo();
        processConnect("gpssensor.ddns.net", "1883");
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
    private void ShowToast(final String message)
    {
        this.runOnUiThread(new Runnable() {
            public void run() {
                Toast.makeText(MainActivity.this, message, Toast.LENGTH_SHORT).show();
            }
        });
    }
    private void processLassInfo(String sMessage,LassInfo li)
    {
        li.clearData();
        String[] arrToken = sMessage.split("\\|");
        for (String item :arrToken) {
            String[] arrAtom = item.split("=");
            if(arrAtom.length!=2)
                continue;
            String sKey=arrAtom[0];
            String sValue=arrAtom[1];
            if(sKey.equals("device_id"))
            {
                li.sDevId=sValue;
            }
            else if(sKey.equals("date"))
            {
                li.sDate=sValue;
            }
            else if(sKey.equals("time"))
            {
                li.sTime=sValue;
            }
            else if(sKey.equals("s_d0"))//pm25
            {
                li.sPM25=sValue;
            }
            else if(sKey.equals("s_t0"))//temperature
            {
                li.sTemp=sValue;
            }
            else if(sKey.equals("s_h0"))//humidity
            {
                li.sHumi=sValue;
            }

        }//end for

    }
    private void _updateUI()
    {
        m_tvDevId.setText(m_li.sDevId);
        m_tvPM25.setText(m_li.sPM25);
        m_tvTemp.setText(m_li.sTemp);
        m_tvHumi.setText(m_li.sHumi);
        m_tvDateTime.setText(m_li.sDate+" "+m_li.sTime);
    }
    private void updateUI()
    {
        String sFilter = m_etFilter.getText().toString().trim();
        if(sFilter.equals("")==false)
        {
            if(m_li.sDevId.toLowerCase().contains(sFilter.toLowerCase())==false)
                return;
        }
        this.runOnUiThread(new Runnable() {
            public void run() {
                _updateUI();
            }
        });
    }
    @Override
    public void connectionLost(Throwable cause) {
        ShowToast("MQTT Connection Lost");
        }

    @Override
    public void messageArrived(String topic, MqttMessage message) throws Exception {
        //ShowToast(topic+":"+message);
        Log.v("LassTest", message.toString());

        String sMessage = message.toString();
        processLassInfo(sMessage,m_li);
        updateUI();
    }

    @Override
    public void deliveryComplete(IMqttDeliveryToken token) {

    }
}
