package com.led.sdk.threads;

import android.util.Log;


import com.led.sdk.wifi.WifiHelper;

import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.MulticastSocket;
import java.net.NetworkInterface;
import java.nio.ByteBuffer;
import java.util.ArrayList;

/**
 * Created by wouters on 8/21/15.
 */
public class TcpRawSend implements Runnable {

    private static final int SERVERPORT = 8080;
    private ByteBuffer mBuffer;
    private int len;
    int speed = 0;
    int b = 0;
    boolean mRunning = false;
    private int mId = 500;

    ArrayList<WifiHelper.IpInfo> mIpList;

    public TcpRawSend(int id, ByteBuffer data) {
        mBuffer = data;
        b = data.get(0);
        mRunning = true;
        mId = id;

       // mIpList = ApManager.getClientList();

    }

    public static String ipToString(int ip, boolean broadcast) {
        String result = new String();

        Integer[] address = new Integer[4];
        for (int i = 0; i < 4; i++)
            address[i] = (ip >> 8 * i) & 0xFF;
        for (int i = 0; i < 4; i++) {
            if (i != 3)
                result = result.concat(address[i] + ".");
            else result = result.concat("255.");
        }
        return result.substring(0, result.length() - 2);
    }

    public void stop() {
        mRunning = false;
    }

    public void setSpeed(int sp) {
        if (sp < 0) {
            sp = 0;
        }
        if (sp >= 50) {
            sp = 50;
        }
        Log.d("WW", "Setting speed to:" + (50 - sp));
        speed = 50 - sp;
    }

    int frame = 0;

    private void sendPacket(MulticastSocket ds, ByteBuffer byteBuff) {
//        if (time == 0 || System.currentTimeMillis() - time > 35) {
//            time = System.currentTimeMillis();
        try {
            DatagramPacket dp;
            if (mId == -3) {
                dp = new DatagramPacket(byteBuff.array(), byteBuff.position(), InetAddress.getByName("192.168.43.1"), 9090);
                Log.v("WW", "Sending out package to id:" + mId);
                ds.send(dp);
                ds.send(dp);
            } else if (mId == -1 || mId == -2) {
                Log.v("WW", "Sending out package to id:" + mId);
                dp = new DatagramPacket(byteBuff.array(), byteBuff.position(), InetAddress.getByName("224.0.0.1"), SERVERPORT);
                ds.send(dp);
                ds.send(dp);
            } else {
                if (mId == 0) {
                    dp = new DatagramPacket(byteBuff.array(), byteBuff.position(), InetAddress.getByName("224.0.0.1"), SERVERPORT);
                    ds.send(dp);
                    dp = new DatagramPacket(byteBuff.array(), byteBuff.position(), InetAddress.getByName("224.0.0.1"), SERVERPORT);
                    ds.send(dp);
                } else {
//                        dp = new DatagramPacket(byteBuff.array(), byteBuff.position(), InetAddress.getByName(temp.ip), SERVERPORT);
//                        ds.send(dp);
//                        dp = new DatagramPacket(byteBuff.array(), byteBuff.position(), InetAddress.getByName(temp.ip), SERVERPORT);
//                        ds.send(dp);
                }
            }
        }catch(Exception e){
            Log.v("Socket Error: ", e.getMessage());
        }
        // }
    }


    private static MulticastSocket ds;
    public void run() {
        try {
            MulticastSocket ds = new MulticastSocket(SERVERPORT);
            if(mId < 1) {
                ds.joinGroup(new InetSocketAddress(InetAddress.getByName("224.0.0.1"), 8080), NetworkInterface.getByName("wlan0"));
                ds.setNetworkInterface(NetworkInterface.getByName("wlan0"));
            }
            sendPacket(ds, mBuffer);
            ds.close();
        } catch (Exception e) {
            e.printStackTrace();
            Log.v("Socket Error: ", "" + e.getMessage());
        }


    }

}
