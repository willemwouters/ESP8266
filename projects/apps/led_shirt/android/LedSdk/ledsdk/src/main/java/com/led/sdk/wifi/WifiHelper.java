package com.led.sdk.wifi;

import android.content.Context;
import android.net.wifi.WifiConfiguration;
import android.util.Log;

import java.io.BufferedReader;
import java.io.FileReader;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;

public class WifiHelper {
    public  static String SSID = "LedW2";
    public  static String PASSWORD = "test12345";
    public final static String NEWSSID = "ESP8266";
    //check whether wifi hotspot on or off

    public static void setUserPass(String user, String pass) {
        SSID = user;
        PASSWORD = pass;
    }

    public static class IpInfo {
        public String mIp;
        public int id = 0;
        public String mMac;
        IpInfo(String ip, String mac, int i) {
            mIp = ip;
            id = i;
            mMac = mac;
        }
    }

    public static ArrayList<IpInfo> getClientList() {
        int macCount = 0;
        BufferedReader br = null;
        ArrayList<IpInfo> arr = new ArrayList<IpInfo>();
        try {
            br = new BufferedReader(new FileReader("/proc/net/arp"));
            String line;
            int c = 0;
            while ((line = br.readLine()) != null) {
                String[] splitted = line.split(" +");
                if (splitted != null ) {
                    // Basic sanity check
                    String mac = splitted[3];
                    if (mac.matches("..:..:..:..:..:..")) {
                        macCount++;
                        c++;
                        arr.add(new IpInfo(splitted[0], mac, c));
                    }
                }
            }
        } catch(Exception e) {

        }
        return arr;
    }

    public static boolean isApOn(Context context) {
        android.net.wifi.WifiManager wifimanager = (android.net.wifi.WifiManager) context.getSystemService(context.WIFI_SERVICE);
        try {
            Method method = wifimanager.getClass().getDeclaredMethod("isWifiApEnabled");
            method.setAccessible(true);


            return (Boolean) method.invoke(wifimanager);
        }
        catch (Throwable ignored) {}
        return false;
    }

    private static String quoteNonHex(String value, int... allowedLengths) {
        return isHexOfLength(value, allowedLengths) ? value : convertToQuotedString(value);
    }
    /**
     * Encloses the incoming string inside double quotes, if it isn't already quoted.
     * @param string the input string
     * @return a quoted string, of the form "input".  If the input string is null, it returns null
     * as well.
     */
    private static String convertToQuotedString(String string) {
        if (string == null || string.length() == 0) {
            return null;
        }
        // If already quoted, return as-is
        if (string.charAt(0) == '"' && string.charAt(string.length() - 1) == '"') {
            return string;
        }
        return '\"' + string + '\"';
    }



    @SuppressWarnings("unchecked")
    private static void setStaticIpConfiguration(android.net.wifi.WifiManager manager, WifiConfiguration config, InetAddress ipAddress, int prefixLength, InetAddress gateway, InetAddress[] dns) throws ClassNotFoundException, IllegalAccessException, IllegalArgumentException, InvocationTargetException, NoSuchMethodException, NoSuchFieldException, InstantiationException
    {
        // First set up IpAssignment to STATIC.
        Object ipAssignment = getEnumValue("android.net.IpConfiguration$IpAssignment", "STATIC");
        callMethod(config, "setIpAssignment", new String[] { "android.net.IpConfiguration$IpAssignment" }, new Object[] { ipAssignment });

        // Then set properties in StaticIpConfiguration.
        Object staticIpConfig = newInstance("android.net.StaticIpConfiguration");
        Object linkAddress = newInstance("android.net.LinkAddress", new Class<?>[] { InetAddress.class, int.class }, new Object[] { ipAddress, prefixLength });

        setField(staticIpConfig, "ipAddress", linkAddress);
        setField(staticIpConfig, "gateway", gateway);
        getField(staticIpConfig, "dnsServers", ArrayList.class).clear();
        for (int i = 0; i < dns.length; i++)
            getField(staticIpConfig, "dnsServers", ArrayList.class).add(dns[i]);

        callMethod(config, "setStaticIpConfiguration", new String[] { "android.net.StaticIpConfiguration" }, new Object[] { staticIpConfig });
        manager.updateNetwork(config);
        manager.saveConfiguration();
    }
    private static Object newInstance(String className) throws ClassNotFoundException, InstantiationException, IllegalAccessException, NoSuchMethodException, IllegalArgumentException, InvocationTargetException
    {
        return newInstance(className, new Class<?>[0], new Object[0]);
    }

    private static Object newInstance(String className, Class<?>[] parameterClasses, Object[] parameterValues) throws NoSuchMethodException, InstantiationException, IllegalAccessException, IllegalArgumentException, InvocationTargetException, ClassNotFoundException
    {
        Class<?> clz = Class.forName(className);
        Constructor<?> constructor = clz.getConstructor(parameterClasses);
        return constructor.newInstance(parameterValues);
    }

    @SuppressWarnings({ "unchecked", "rawtypes" })
    private static Object getEnumValue(String enumClassName, String enumValue) throws ClassNotFoundException
    {
        Class<Enum> enumClz = (Class<Enum>)Class.forName(enumClassName);
        return Enum.valueOf(enumClz, enumValue);
    }

    private static void setField(Object object, String fieldName, Object value) throws IllegalAccessException, IllegalArgumentException, NoSuchFieldException
    {
        Field field = object.getClass().getDeclaredField(fieldName);
        field.set(object, value);
    }

    private static <T> T getField(Object object, String fieldName, Class<T> type) throws IllegalAccessException, IllegalArgumentException, NoSuchFieldException
    {
        Field field = object.getClass().getDeclaredField(fieldName);
        return type.cast(field.get(object));
    }

    private static void callMethod(Object object, String methodName, String[] parameterTypes, Object[] parameterValues) throws ClassNotFoundException, IllegalAccessException, IllegalArgumentException, InvocationTargetException, NoSuchMethodException
    {
        Class<?>[] parameterClasses = new Class<?>[parameterTypes.length];
        for (int i = 0; i < parameterTypes.length; i++)
            parameterClasses[i] = Class.forName(parameterTypes[i]);

        Method method = object.getClass().getDeclaredMethod(methodName, parameterClasses);
        method.invoke(object, parameterValues);
    }
    /**
     * @param value input to check
     * @param allowedLengths allowed lengths, if any
     * @return true if value is a non-null, non-empty string of hex digits, and if allowed lengths are given, has
     *  an allowed length
     */
    private static final Pattern HEX_DIGITS = Pattern.compile("[0-9A-Fa-f]+");

    private static boolean isHexOfLength(CharSequence value, int... allowedLengths) {
        if (value == null || !HEX_DIGITS.matcher(value).matches()) {
            return false;
        }
        if (allowedLengths.length == 0) {
            return true;
        }
        for (int length : allowedLengths) {
            if (value.length() == length) {
                return true;
            }
        }
        return false;
    }
    private static WifiConfiguration changeNetworkCommon(String ssid) {
        WifiConfiguration config = new WifiConfiguration();
        config.allowedAuthAlgorithms.clear();
        config.allowedGroupCiphers.clear();
        config.allowedKeyManagement.clear();
        config.allowedPairwiseCiphers.clear();
        config.allowedProtocols.clear();
        // Android API insists that an ascii SSID must be quoted to be correctly handled.
        config.SSID = ssid; // quoteNonHex(ssid);
        return config;
    }

    public static WifiConfiguration changeNetworkWPA(android.net.wifi.WifiManager wifiManager, String ssid, String password) {
        WifiConfiguration config = changeNetworkCommon(ssid);
        // Hex passwords that are 64 bits long are not to be quoted.
        config.preSharedKey = password; //quoteNonHex(password, 64);
        //config.allowedAuthAlgorithms.set(WifiConfiguration.AuthAlgorithm.OPEN);
        config.allowedProtocols.set(WifiConfiguration.Protocol.WPA); // For WPA
        config.allowedProtocols.set(WifiConfiguration.Protocol.RSN); // For WPA2
        config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_PSK);
        config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_EAP);
        config.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.TKIP);
        config.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.CCMP);
        config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.TKIP);
        config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.CCMP);
        return config;
    }

    public static void connectToAp(Context context, String ssid, String networkPass) {

        WifiConfiguration conf = new WifiConfiguration();
        conf.SSID = String.format("\"%s\"", ssid);
        conf.preSharedKey = String.format("\"%s\"", networkPass);
        conf.allowedProtocols.set(WifiConfiguration.Protocol.WPA); // For WPA
        conf.allowedProtocols.set(WifiConfiguration.Protocol.RSN); // For WPA2
        conf.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_PSK);
        conf.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_EAP);
        conf.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.TKIP);
        conf.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.CCMP);
        conf.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.TKIP);
        conf.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.CCMP);
        android.net.wifi.WifiManager wifiManager = (android.net.wifi.WifiManager)context.getSystemService(Context.WIFI_SERVICE);
        wifiManager.addNetwork(conf);

        List<WifiConfiguration> list = wifiManager.getConfiguredNetworks();
        for( WifiConfiguration i : list ) {
            if(i.SSID != null && i.SSID.equals("\"" + ssid + "\"")) {
                wifiManager.disconnect();
                wifiManager.enableNetwork(i.networkId, true);
                wifiManager.reconnect();
                break;
            }
        }
    }
    // toggle wifi hotspot on or off
    public static boolean configApState(Context context, boolean enabled, String ssid, String password) {
        android.net.wifi.WifiManager wifimanager = (android.net.wifi.WifiManager) context.getSystemService(context.WIFI_SERVICE);
        WifiConfiguration conf = changeNetworkWPA(wifimanager, ssid, password);
        try {
            wifimanager.setWifiEnabled(false);

            wifimanager.updateNetwork(conf); //apply the setting
            wifimanager.saveConfiguration(); //Save it

            Method method = wifimanager.getClass().getMethod("setWifiApEnabled", WifiConfiguration.class, boolean.class);
            method.invoke(wifimanager, conf, enabled);
            return true;
        }
        catch (Exception e) {
            e.printStackTrace();
            System.out.println("IO EXcept");
        }
        return false;
    }
} // end of class