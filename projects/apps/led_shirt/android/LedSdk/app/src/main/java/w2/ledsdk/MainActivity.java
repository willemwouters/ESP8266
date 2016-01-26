package w2.ledsdk;

import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.SeekBar;
import android.widget.ToggleButton;

import com.led.sdk.driver.Framedriver;
import com.led.sdk.wifi.WifiHelper;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    private static String mApSsid = "TestAp";
    private static String mApPassword = "TestAp";
    private boolean mFade = false;
    private boolean mFlicker = false;

    private Context mContext;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext = this;
        setContentView(R.layout.activity_main);

        Button lBtn = (Button) findViewById(R.id.btnApOn);
        lBtn.setOnClickListener(this);

        lBtn = (Button) findViewById(R.id.btnApOff);
        lBtn.setOnClickListener(this);

        lBtn = (Button) findViewById(R.id.btnApConfigSsid);
        lBtn.setOnClickListener(this);

        lBtn = (Button) findViewById(R.id.btnConfigureConnected);
        lBtn.setOnClickListener(this);

        lBtn = (Button) findViewById(R.id.btnSendText);
        lBtn.setOnClickListener(this);

        lBtn = (Button) findViewById(R.id.btnSendBrightness);
        lBtn.setOnClickListener(this);

        lBtn = (Button) findViewById(R.id.btnSendFade);
        lBtn.setOnClickListener(this);

        lBtn = (Button) findViewById(R.id.btnSendFlicker);
        lBtn.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btnApOn:
                WifiHelper.configApState(this, true, mApSsid, mApPassword);
                break;
            case R.id.btnApOff:
                WifiHelper.configApState(this, false, mApSsid, mApPassword);
                break;
            case R.id.btnApConfigSsid:
                configureApSsid();
                break;
            case R.id.btnConfigureConnected:
                Framedriver.setAp(mApSsid, mApPassword);
                break;
            case R.id.btnSendBrightness:
                sendBrightness();
                break;
            case R.id.btnSendFade:
                sendFade();
                break;
            case R.id.btnSendFlicker:
                sendFlicker();
                break;
            case R.id.btnSendText:
                sendText();
                break;
            default:
                break;
        }
    }

    private void sendBrightness() {
        LayoutInflater li = LayoutInflater.from(mContext);
        View promptsView = li.inflate(R.layout.prompts_seek, null);
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(mContext);
        alertDialogBuilder.setView(promptsView);
        final SeekBar userInput = (SeekBar) promptsView.findViewById(R.id.progressBarDialogUserInput);
        userInput.setMax(250);
        alertDialogBuilder
                .setCancelable(false)
                .setTitle("Brightness:")
                .setPositiveButton("Send", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog,int id) {
                        Framedriver.setBrightness(0, userInput.getProgress());
                    }
                });
        AlertDialog alertDialog = alertDialogBuilder.create();
        alertDialog.show();
    }

    private void sendFlicker() {
        LayoutInflater li = LayoutInflater.from(mContext);
        View promptsView = li.inflate(R.layout.prompts_toggle_seek, null);
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(mContext);
        alertDialogBuilder.setView(promptsView);
        final SeekBar userInput = (SeekBar) promptsView.findViewById(R.id.progressBarDialogUserInput);
        final ToggleButton userButton = (ToggleButton) promptsView.findViewById(R.id.toggleDialogUserInput);
        userInput.setMax(250);
        alertDialogBuilder
                .setCancelable(false)
                .setTitle("Brightness:")
                .setPositiveButton("Send", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog,int id) {
                        Framedriver.setFramespeed(0, userInput.getProgress());
                        Framedriver.setFlicker(0, userButton.isChecked());
                    }
                });
        AlertDialog alertDialog = alertDialogBuilder.create();
        alertDialog.show();
    }

    private void sendFade() {
        LayoutInflater li = LayoutInflater.from(mContext);
        View promptsView = li.inflate(R.layout.prompts_toggle_seek, null);
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(mContext);
        alertDialogBuilder.setView(promptsView);
        final SeekBar userInput = (SeekBar) promptsView.findViewById(R.id.progressBarDialogUserInput);
        final ToggleButton userButton = (ToggleButton) promptsView.findViewById(R.id.toggleDialogUserInput);
        userInput.setMax(250);
        alertDialogBuilder
                .setCancelable(false)
                .setTitle("Brightness:")
                .setPositiveButton("Send", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog,int id) {
                        Framedriver.setFramespeedFade(0, userInput.getProgress());
                        Framedriver.setFlickerFade(0, userButton.isChecked());
                    }
                });
        AlertDialog alertDialog = alertDialogBuilder.create();
        alertDialog.show();
    }

    private void sendText() {
        LayoutInflater li = LayoutInflater.from(mContext);
        View promptsView = li.inflate(R.layout.prompts_edit, null);
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(mContext);
        alertDialogBuilder.setView(promptsView);
        final EditText userInput = (EditText) promptsView.findViewById(R.id.editTextDialogUserInput);
        alertDialogBuilder
                .setCancelable(false)
                .setTitle("Text:")
                .setPositiveButton("Send", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog,int id) {
                        Framedriver.setText(0, userInput.getText().toString(), true);
                    }
                });
        AlertDialog alertDialog = alertDialogBuilder.create();
        alertDialog.show();
    }

    private void configureApSsid() {
        LayoutInflater li = LayoutInflater.from(mContext);
        View promptsView = li.inflate(R.layout.prompts_editdual, null);
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(mContext);
        alertDialogBuilder.setView(promptsView);
        final EditText userInput1 = (EditText) promptsView.findViewById(R.id.editTextDialogUserInput1);
        final EditText userInput2 = (EditText) promptsView.findViewById(R.id.editTextDialogUserInput2);
        alertDialogBuilder
                .setCancelable(false)
                .setTitle("Ap SSID")
                .setPositiveButton("Next", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog,int id) {
                        mApSsid = userInput1.getText().toString();
                        mApPassword = userInput2.getText().toString();
                    }
                });
        AlertDialog alertDialog = alertDialogBuilder.create();
        alertDialog.show();
    }

}
