package org.lanma.lassandroid;

import android.app.Activity;
import android.content.Intent;
import android.content.IntentSender;
import android.location.Location;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.api.GoogleApiClient;
import com.google.android.gms.common.api.GoogleApiClient.ConnectionCallbacks;
import com.google.android.gms.common.api.GoogleApiClient.OnConnectionFailedListener;
import com.google.android.gms.common.api.PendingResult;
import com.google.android.gms.common.api.ResultCallback;

import com.google.android.gms.common.api.Status;
import com.google.android.gms.location.LocationRequest;
import com.google.android.gms.location.LocationServices;
import com.google.android.gms.location.LocationSettingsRequest;
import com.google.android.gms.location.LocationSettingsResult;
import com.google.android.gms.location.LocationListener;
import com.google.android.gms.location.LocationSettingsStatusCodes;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;

import java.text.DateFormat;
import java.util.Date;

public class MainActivity extends AppCompatActivity implements
        MqttCallback,
        ConnectionCallbacks,
        OnConnectionFailedListener,
        LocationListener,
        ResultCallback<LocationSettingsResult> {
    protected static final String TAG = "MainActivity";
    protected static final int REQUEST_CHECK_SETTINGS = 0x1;
    public static final long UPDATE_INTERVAL_IN_MILLISECONDS = 10000;
    public static final long FASTEST_UPDATE_INTERVAL_IN_MILLISECONDS =
            UPDATE_INTERVAL_IN_MILLISECONDS / 2;

    protected GoogleApiClient mGoogleApiClient;
    protected LocationRequest mLocationRequest;
    protected LocationSettingsRequest mLocationSettingsRequest;
    protected Location mCurrentLocation;
    protected Boolean mRequestingLocationUpdates;
    protected String mLastUpdateTime;

    private static String clientId = "LassAndroid";
    private static MqttClient mqttClient;
    //public static final String TOPIC = "TurtleCar";
    public static final String TOPIC_STATUS = "LASS/Test/PM25";
    public static final String DEVICE_ID = "LANMA_001";
    public static final int QOS = 0;
    public static final int TIMEOUT = 3;

    private org.lanma.lassandroid.LassInfo m_li;
    TextView m_tvDevId;
    TextView m_tvPM25;
    TextView m_tvTemp;
    TextView m_tvHumi;
    TextView m_tvDateTime;
    TextView m_tvLoc;
    TextView m_tvMyLoc;
    EditText m_etFilter;
    private void initUI()
    {
        m_tvDevId = (TextView)this.findViewById(org.lanma.lassandroid.R.id.tvDevId);
        m_tvPM25 = (TextView)this.findViewById(org.lanma.lassandroid.R.id.tvPM25);
        m_tvTemp = (TextView)this.findViewById(org.lanma.lassandroid.R.id.tvTemp);
        m_tvHumi = (TextView)this.findViewById(org.lanma.lassandroid.R.id.tvHumi);
        m_tvDateTime = (TextView)this.findViewById(org.lanma.lassandroid.R.id.tvDateTime);
        m_tvLoc = (TextView)this.findViewById(org.lanma.lassandroid.R.id.tvLoc);
        m_tvMyLoc = (TextView)this.findViewById(org.lanma.lassandroid.R.id.tvMyLoc);
        m_etFilter = (EditText)this.findViewById(org.lanma.lassandroid.R.id.etFilter);
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
        setContentView(org.lanma.lassandroid.R.layout.activity_main);

        mRequestingLocationUpdates=false;
        buildGoogleApiClient();
        createLocationRequest();
        buildLocationSettingsRequest();


        initUI();
        m_li = new org.lanma.lassandroid.LassInfo();
        processConnect("gpssensor.ddns.net", "1883");

        checkLocationSettings();
        //stopLocationUpdates();
    }

    @Override
    protected void onStart() {
        super.onStart();
        mGoogleApiClient.connect();

    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mGoogleApiClient.isConnected() && mRequestingLocationUpdates) {
            startLocationUpdates();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (mGoogleApiClient.isConnected()) {
            stopLocationUpdates();
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        mGoogleApiClient.disconnect();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(org.lanma.lassandroid.R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == org.lanma.lassandroid.R.id.action_settings) {
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
    private void processLassInfo(String sMessage, org.lanma.lassandroid.LassInfo li)
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
            else if(sKey.equals("gps_lat"))//lat
            {
                li.sLat=sValue;
            }
            else if(sKey.equals("gps_lon"))//lng
            {
                li.sLng=sValue;
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
        m_tvLoc.setText(m_li.sLat+","+m_li.sLng);
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
        processLassInfo(sMessage, m_li);
        updateUI();
    }

    @Override
    public void deliveryComplete(IMqttDeliveryToken token) {

    }

    protected synchronized void buildGoogleApiClient() {
        Log.i(TAG, "Building GoogleApiClient");
        mGoogleApiClient = new GoogleApiClient.Builder(this)
                .addConnectionCallbacks(this)
                .addOnConnectionFailedListener(this)
                .addApi(LocationServices.API)
                .build();
    }
    protected void createLocationRequest() {
        mLocationRequest = new LocationRequest();
        mLocationRequest.setInterval(UPDATE_INTERVAL_IN_MILLISECONDS);
        mLocationRequest.setFastestInterval(FASTEST_UPDATE_INTERVAL_IN_MILLISECONDS);

        mLocationRequest.setPriority(LocationRequest.PRIORITY_HIGH_ACCURACY);
    }
    protected void buildLocationSettingsRequest() {
        LocationSettingsRequest.Builder builder = new LocationSettingsRequest.Builder();
        builder.addLocationRequest(mLocationRequest);
        mLocationSettingsRequest = builder.build();
    }


    private void updateLocationUI() {
        if (mCurrentLocation != null) {
            if (mCurrentLocation != null) {
                m_tvMyLoc.setText(String.format("%.4f,%.4f",
                        mCurrentLocation.getLatitude(),
                        mCurrentLocation.getLongitude()));
                //mLastUpdateTimeTextView.setText(String.format("%s: %s", mLastUpdateTimeLabel,
                //        mLastUpdateTime));
            }
        }
    }

    /**
     * Callback that fires when the location changes.
     */
    @Override
    public void onLocationChanged(Location location) {
        mCurrentLocation = location;
        mLastUpdateTime = DateFormat.getTimeInstance().format(new Date());
        updateLocationUI();
        Toast.makeText(this, "Location Updated",
                Toast.LENGTH_SHORT).show();
        String payload = String.format("|gps_lat=%f|gps_lon=%f"
                            ,location.getLatitude(),location.getLongitude());
        MqttMessage qm = new MqttMessage();
        qm.setPayload(payload.getBytes());
        try {
            mqttClient.publish(TOPIC_STATUS + "/" + DEVICE_ID + "/" + "C",qm);
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onConnected(Bundle bundle) {
        if (mCurrentLocation == null) {
            mCurrentLocation = LocationServices.FusedLocationApi.getLastLocation(mGoogleApiClient);
            mLastUpdateTime = DateFormat.getTimeInstance().format(new Date());
            updateLocationUI();
        }
    }

    @Override
    public void onConnectionSuspended(int cause) {
        Log.i(TAG, "Connection suspended");
    }

    @Override
    public void onConnectionFailed(ConnectionResult result) {
        // Refer to the javadoc for ConnectionResult to see what error codes might be returned in
        // onConnectionFailed.
        Log.i(TAG, "Connection failed: ConnectionResult.getErrorCode() = " + result.getErrorCode());
    }

    @Override
    public void onResult(LocationSettingsResult locationSettingsResult) {
        final Status status = locationSettingsResult.getStatus();
        switch (status.getStatusCode()) {
            case LocationSettingsStatusCodes.SUCCESS:
                Log.i(TAG, "All location settings are satisfied.");
                startLocationUpdates();
                break;
            case LocationSettingsStatusCodes.RESOLUTION_REQUIRED:
                Log.i(TAG, "Location settings are not satisfied. Show the user a dialog to" +
                        "upgrade location settings ");

                try {
                    // Show the dialog by calling startResolutionForResult(), and check the result
                    // in onActivityResult().
                    status.startResolutionForResult(MainActivity.this, REQUEST_CHECK_SETTINGS);
                } catch (IntentSender.SendIntentException e) {
                    Log.i(TAG, "PendingIntent unable to execute request.");
                }
                break;
            case LocationSettingsStatusCodes.SETTINGS_CHANGE_UNAVAILABLE:
                Log.i(TAG, "Location settings are inadequate, and cannot be fixed here. Dialog " +
                        "not created.");
                break;
        }

    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        switch (requestCode) {
            // Check for the integer request code originally supplied to startResolutionForResult().
            case REQUEST_CHECK_SETTINGS:
                switch (resultCode) {
                    case Activity.RESULT_OK:
                        Log.i(TAG, "User agreed to make required location settings changes.");
                        startLocationUpdates();
                        break;
                    case Activity.RESULT_CANCELED:
                        Log.i(TAG, "User chose not to make required location settings changes.");
                        break;
                }
                break;
        }
    }

    protected void checkLocationSettings() {
        PendingResult<LocationSettingsResult> result =
                LocationServices.SettingsApi.checkLocationSettings(
                        mGoogleApiClient,
                        mLocationSettingsRequest
                );
        result.setResultCallback(this);
    }
    protected void startLocationUpdates() {
        LocationServices.FusedLocationApi.requestLocationUpdates(
                mGoogleApiClient,
                mLocationRequest,
                this
        ).setResultCallback(new ResultCallback<Status>() {
            @Override
            public void onResult(Status status) {
                mRequestingLocationUpdates = true;
                //setButtonsEnabledState();
            }
        });

    }

    protected void stopLocationUpdates() {
        // It is a good practice to remove location requests when the activity is in a paused or
        // stopped state. Doing so helps battery performance and is especially
        // recommended in applications that request frequent location updates.
        LocationServices.FusedLocationApi.removeLocationUpdates(
                mGoogleApiClient,
                this
        ).setResultCallback(new ResultCallback<Status>() {
            @Override
            public void onResult(Status status) {
                mRequestingLocationUpdates = false;
                //setButtonsEnabledState();
            }
        });
    }

}
