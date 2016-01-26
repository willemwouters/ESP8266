package com.led.sdk.driver;

import android.os.Handler;
import android.os.Looper;


import com.led.sdk.threads.TcpRawSend;
import com.led.sdk.threads.TcpSend;

import java.nio.ByteBuffer;
import java.util.ArrayList;

/**
 * Created by wouters on 9/5/15.
 */
public class Framedriver {

    public final static int REG_TEXTWRITE = 0x00;
    public final static int REG_STREAMWRITE = 0x01;
    public final static int REG_FLICKERMODE = 0x02;
    public final static int REG_FLICKERFADE = 0x0A;
    public final static int REG_FLICKERBUFFERMODE = 0x03;
    public final static int REG_FRAMESPEED = 0x04;
    public final static int REG_FRAMESPEEDTEXT = 0x0B;
    public final static int REG_FRAMESPEEDFADE = 0x0C;
    public final static int REG_FRAMESPEEDICON = 0x0D;
    public final static int REG_MULTICASTLOCK = 0x0E;
    public final static int REG_KEYPRESS = 0x0F;
    public final static int REG_BRIGHTNESS = 0x05;
    public final static int REG_SETAP = 0xFC;
    public final static int REG_SETVAL = 0x07;
    public final static int REG_FONTCOLORFRONT = 0x08;
    public final static int REG_FONTCOLORBACK = 0x09;


    private static int sFrontColorR = 0xFF;
    private static int sFrontColorG = 0xFF;
    private static int sFrontColorB = 0xFF;

    private static void sendSimpleVal(final int id, int val, final int type) {
        final ArrayList<Byte> b = new ArrayList<Byte>();
        b.add((byte) val);
        b.add((byte) 0);
        b.add((byte) 0);
        b.add((byte) 0);
        new Thread(new TcpSend(id, b, type, 0)).start();
    }

    public static void setTextForward(int id, String text, boolean upper) {
        ArrayList<Byte> b = new ArrayList<Byte>();
        int count = 0;
        b.add((byte) 0); // scoll disabled
        for (int x = 0; x < text.length(); x++) {
            byte r = 0;
            if(upper) {
                r = (byte) text.toUpperCase().charAt(x);
            } else {
                r = (byte) text.charAt(x);
            }
            b.add(r);
        }
        new Thread(new TcpSend(id, b, REG_TEXTWRITE, 0)).start();
    }

    public static void setText(int id, String text, boolean upper) {
        ArrayList<Byte> b = new ArrayList<Byte>();
        int count = 0;
        b.add((byte) 1); // scoll enabled
        b.add((byte) ' ');
        for (int x = 0; x < text.length(); x++) {
            byte r = 0;
            if(upper) {
                r = (byte) text.toUpperCase().charAt(x);
            } else {
                r = (byte) text.charAt(x);
            }
            b.add(r);
        }
        new Thread(new TcpSend(id, b, REG_TEXTWRITE, 0)).start();


    }

    public static void setAp(String ap, String password) {
        final ArrayList<Byte> buf = new ArrayList<Byte>();
        buf.add((byte) ap.length());
        buf.add((byte) password.length());
        for(int i = 0; i < ap.length(); i++) {
            buf.add((byte) ap.charAt(i));
        }
        for(int i = 0; i < password.length(); i++) {
            buf.add((byte) password.charAt(i));
        }
        new Thread(new TcpSend(0, buf, REG_SETAP, 0)).start();
    }

    public static void setValue(final int id, int buffer, int r, int g, final int b) {
        final ArrayList<Byte> buf = new ArrayList<Byte>();
        buf.add((byte) buffer);
        buf.add((byte) r);
        buf.add((byte) g);
        buf.add((byte) b);
        new Handler(Looper.getMainLooper()).postDelayed(new Runnable() {
            @Override
            public void run() {
                new Thread(new TcpSend(id, buf, REG_SETVAL, 0)).start();
            }
        }, 10);
        new Thread(new TcpSend(id, buf, REG_SETVAL, 0)).start();
    }

    public static void setLock(final int id, boolean val) {
        final ArrayList<Byte> b = new ArrayList<Byte>();
        if (val) {
            b.add((byte) 1);
        } else {
            b.add((byte) 0);
        }
        b.add((byte) 0);
        b.add((byte) 0);
        b.add((byte) 0);
        new Thread(new TcpSend(id, b, REG_MULTICASTLOCK, 0)).start();
    }

    public static void setFlickerFade(final int id, boolean val) {
        final ArrayList<Byte> b = new ArrayList<Byte>();
        if (val) {
            b.add((byte) 0);
        } else {
            b.add((byte) 1);
        }
        b.add((byte) 0);
        b.add((byte) 0);
        b.add((byte) 0);
        new Thread(new TcpSend(id, b, REG_FLICKERFADE, 0)).start();

    }

    public static void setFlicker(final int id, boolean val) {
        final ArrayList<Byte> b = new ArrayList<Byte>();
        if (val) {
            b.add((byte) 0);
        } else {
            b.add((byte) 1);
        }
        b.add((byte) 0);
        b.add((byte) 0);
        b.add((byte) 0);
        new Thread(new TcpSend(id, b, REG_FLICKERMODE, 0)).start();
    }

    public static void setFlickerBuffer(final int id, boolean val) {
        final ArrayList<Byte> b = new ArrayList<Byte>();
        if (val) {
            b.add((byte) 0);
        } else {
            b.add((byte) 1);
        }
        b.add((byte) 0);
        b.add((byte) 0);
        b.add((byte) 0);
        new Thread(new TcpSend(id, b, REG_FLICKERBUFFERMODE, 0)).start();
    }

    public static void setColorTextBack(int id, int r, int g, int b) {
        ArrayList<Byte> buf = new ArrayList<Byte>();
        buf.add((byte) r);
        buf.add((byte) g);
        buf.add((byte) b);
        new Thread(new TcpSend(id, buf, REG_FONTCOLORBACK, 0)).start();
    }

    public static void setColorTextFront(int id, int r, int g, int b) {
        ArrayList<Byte> buf = new ArrayList<Byte>();
        buf.add((byte) r);
        sFrontColorR = r;
        buf.add((byte) g);
        sFrontColorG = g;
        buf.add((byte) b);
        sFrontColorB = b;
        new Thread(new TcpSend(id, buf, REG_FONTCOLORFRONT, 0)).start();
    }

    public static void setFramespeed(int id, int speed) {
        sendSimpleVal(id, speed + 1, REG_FRAMESPEED);
    }
    public static void setFramespeedFade(int id, int speed) {
        sendSimpleVal(id, speed + 1, REG_FRAMESPEEDFADE);
    }

    public static void setFramespeedText(int id, int speed) {
        sendSimpleVal(id, speed+1, REG_FRAMESPEEDTEXT);
    }

    public static void setFramespeedIcon(int id, int speed) {
        sendSimpleVal(id, speed+1, REG_FRAMESPEEDICON);
    }

    public static void setBrightness(int id, int bright) {
        if (bright == 0) {
            bright = 1;
        }
        sendSimpleVal(id, bright, REG_BRIGHTNESS);
    }

    public static void sendBuffer(final int id, final ByteBuffer buf) {
        new Thread(new TcpRawSend(id, buf)).start();
    }



}
