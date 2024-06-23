package com.example.smarthomeapp;
import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";
    private static final String HC05_MAC_ADDRESS = "A2:2A:09:07:0F:63";
    private static final UUID HC05_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    private static final int REQUEST_BLUETOOTH_PERMISSIONS = 1;

    private BluetoothAdapter bluetoothAdapter;
    private BluetoothSocket bluetoothSocket;
    private Button btnConnect;
    private Button btnLed1On, btnLed1Off, btnLed2On, btnLed2Off, btnLed3On, btnLed3Off;
    private TextView tvSensorData;
    private Handler handler;
    private OutputStream outputStream;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        btnConnect = findViewById(R.id.btnConnect);
        btnLed1On = findViewById(R.id.btnLed1On);
        btnLed1Off = findViewById(R.id.btnLed1Off);
        btnLed2On = findViewById(R.id.btnLed2On);
        btnLed2Off = findViewById(R.id.btnLed2Off);
        btnLed3On = findViewById(R.id.btnLed3On);
        btnLed3Off = findViewById(R.id.btnLed3Off);
        tvSensorData = findViewById(R.id.tvSensorData);
        handler = new Handler();

        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

        btnConnect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                checkPermissionsAndConnect();
            }
        });

        btnLed1On.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendCommand("1");
            }
        });

        btnLed1Off.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendCommand("2");
            }
        });

        btnLed2On.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendCommand("3");
            }
        });

        btnLed2Off.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendCommand("4");
            }
        });
        btnLed3On.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendCommand("5");
            }
        });
        btnLed3Off.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendCommand("6");
            }
        });
    }

    private void checkPermissionsAndConnect() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.BLUETOOTH_CONNECT}, REQUEST_BLUETOOTH_PERMISSIONS);
            } else {
                connectToHC05();
            }
        } else {
            connectToHC05();
        }
    }

    private void connectToHC05() {
        BluetoothDevice hc05 = bluetoothAdapter.getRemoteDevice(HC05_MAC_ADDRESS);

        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    bluetoothSocket = hc05.createRfcommSocketToServiceRecord(HC05_UUID);
                    bluetoothSocket.connect();
                    outputStream = bluetoothSocket.getOutputStream();
                    readSensorData();
                } catch (IOException e) {
                    Log.e(TAG, "Connection failed", e);
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            Toast.makeText(MainActivity.this, "Connection failed", Toast.LENGTH_SHORT).show();
                        }
                    });
                }
            }
        }).start();
    }

    private void sendCommand(String command) {
        if (outputStream != null) {
            try {
                outputStream.write(command.getBytes());
            } catch (IOException e) {
                Log.e(TAG, "Error sending command", e);
            }
        }
    }

    private void readSensorData() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                InputStream inputStream;
                try {
                    inputStream = bluetoothSocket.getInputStream();
                    byte[] buffer = new byte[1024];
                    int bytes;

                    while (true) {
                        try {
                            bytes = inputStream.read(buffer);
                            final String readMessage = new String(buffer, 0, bytes);
                            handler.post(new Runnable() {
                                @Override
                                public void run() {
                                    tvSensorData.setText(readMessage);
                                }
                            });
                        } catch (IOException e) {
                            Log.e(TAG, "Error reading data", e);
                            break;
                        }
                    }
                } catch (IOException e) {
                    Log.e(TAG, "Error getting input stream", e);
                }
            }
        }).start();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        try {
            if (bluetoothSocket != null) {
                bluetoothSocket.close();
            }
        } catch (IOException e) {
            Log.e(TAG, "Error closing socket", e);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_BLUETOOTH_PERMISSIONS) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                connectToHC05();
            } else {
                Toast.makeText(this, "Bluetooth permissions are required to connect to the HC-05 module.", Toast.LENGTH_SHORT).show();
            }
        }
    }
}