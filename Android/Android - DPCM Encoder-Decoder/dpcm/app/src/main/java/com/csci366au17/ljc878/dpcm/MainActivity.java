package com.csci366au17.ljc878.dpcm;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import java.io.InputStream;
import java.util.StringTokenizer;

public class MainActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        String notification = "Please Enter samples separated by a comma and push (Encode/Decode) to process the data. Now whitespaces preferred.";
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toast.makeText(getApplicationContext(), notification, Toast.LENGTH_LONG).show();
    }

    void doEncoding(View v) {//TODO Test
        TextView textView = (TextView) findViewById(R.id.textView);
        int[] inputData;
        int[] quantizedError;
        //get the data into an array
        //for each value but the first
        //get the predicted value
        //get the error by subtracting the input from the predicted
        //quantize the error
        inputData = getInput();
        quantizedError = new int[inputData.length];
        int count = inputData.length;
        if (inputData[1] == 0) {
            //then there is an error with the input
            //break
        } else {
            //process as normal
            int error = 0;
            int predicted = 0;
            int temp = 0;
            for (int i = 0; i < quantizedError.length; i++) {
                if (i == 0) {
                    quantizedError[i] = inputData[i];
                } //for the first value do nothing
                if (i == 1) {
                    predicted = (quantizedError[1] + quantizedError[1]) / 2;
                    error = inputData[i] - predicted;
                    //trunc is redundant as int already rounds towards zero
                    temp = ((255 - error) / 16);
                    temp = 16 * temp - 256+8;
                    quantizedError[i] = temp;
                }

            }
            StringBuilder input = new StringBuilder();
            StringBuilder outputq = new StringBuilder();
            StringBuilder outputr = new StringBuilder();
            StringBuilder snr = new StringBuilder();
            float SNR =0;
            int S = 0;
            int N = 0;
            //TODO
            //calculate the SNR
            // A SNR= 20log10(2^n)
            // B SNR = difference between the quantized values and the original
            // D SNR = 6.02N+1.76dB
            // where n is the difference between the original and the quantized
            //put the values into strings
            for (int x = 0; x < count; x++) {
                outputq.append(quantizedError[x] + " ");
                N+=quantizedError[x];
                input.append(inputData[x] + " ");
                S+=inputData[x];
                if(x==0){
                    outputr.append(quantizedError[x] + " ");
                }
                else {
                    int c = quantizedError[x]+ quantizedError[x-1];
                    outputr.append(c + " ");
                }
            }
            SNR = (S/N) * 6.02f + 1.76f;
            snr.append(SNR);
            textView.setText("Input fn: " + input.toString() + "\nOutput ~en: " + outputq.toString() + "\nOutput ~fn:" + outputr.toString() + "\nSNR: " + snr.toString());

        }

    }

    void doDecoding(View v) {
        /*
        reconstruct the values using the formula
        r1 = f1, r2 = f1 +e2, r3 = r2 +e3 ...
        get all the values into an array
        for (each value) < last value) {
            if(value[i] > 0 && value[i] < 255
                value[i-1] + value[i]
                store in output array
            else
                number outside range
         }
         Transform output into string
         display output in Textview
         */
        String temp = "";
        TextView textView = (TextView) findViewById(R.id.textView);
        EditText edittext = (EditText) findViewById(R.id.editText);
        String InputString = edittext.getText().toString();
        String[] splitstring = InputString.split(",");

        /*Actual Method For string to int to string
        String test = edittext.getText().toString();
        String[] split = test.split(",");
        test = split[0];
        int p = Integer.parseInt(test);
        int q = p + Integer.parseInt(split[1]);
        //Build the values into a StringBuilder
        StringBuilder builder = new StringBuilder();
        builder.append("Value 1:" + p + "\n" + "Value 2: " + q);
        //call .toString() on StringBuilder()
        textView.setText(builder.toString());
        /*Method End
        */
        int count = splitstring.length;
        int[] retval = new int[count];
        for (int i = 0; i < count; i++) {
            temp = splitstring[i];
            if (temp != null && !temp.isEmpty()) {
                String nowhitespace = temp.trim();
                int checker = Integer.parseInt(nowhitespace);
                if (checker >= 0 || checker <= 255) {
                    //what if the value is 0-255
                    retval[i] = retval[i - 1] + checker;
                } else {
                    Toast.makeText(getApplicationContext(), "There was a number outside the 0-255 range entered please star over", Toast.LENGTH_LONG).show();
                    return;
                }
                //make the string empty
                temp = "";
            } else {
                Toast.makeText(getApplicationContext(), "There was an empty string", Toast.LENGTH_LONG).show();
            }
        }
        //Display Input and Output Values int the TextView
        StringBuilder input = new StringBuilder();
        StringBuilder output = new StringBuilder();
        //put the reconstructed values into a string to display them on the screen
        for (int x = 0; x < count; x++) {
            output.append(retval[x] + " ");
            input.append(splitstring[x] + " ");
        }
        textView.setText("Input ~en: " + input.toString() + "\nOutput ~fn: " + output.toString());

    }


    int[] getInput() {//TODO Test
        //input will be as unsigned 8bit samples
        //8bit = 1 byte //byte to int = 0xFF & input
        String temp = "";
        TextView textView = (TextView) findViewById(R.id.textView);
        EditText edittext = (EditText) findViewById(R.id.editText);
        String InputString = edittext.getText().toString();
        String[] splitstring = InputString.split(",");

        int count = splitstring.length;
        int[] retval = new int[count];
        for (int i = 0; i < count; i++) {
            temp = splitstring[i];
            if (temp != null && !temp.isEmpty()) {
                String nowhitespace = temp.trim();
                int checker = Integer.parseInt(nowhitespace,2);
                if (checker >= 0 && checker <= 255) {
                    //if the value is 0-255
                    if(i>0)
                        retval[i] = retval[i - 1] + checker;
                    else
                        retval[i] = checker;
                } else {
                    Toast.makeText(getApplicationContext(), "There was a number outside the 0-255 range entered please star over", Toast.LENGTH_LONG).show();
                    retval[0] = 0;
                    return retval;
                }
                //make the string empty
                temp = "";
            } else {
                Toast.makeText(getApplicationContext(), "There was an empty string", Toast.LENGTH_LONG).show();
                retval[0] =0;
                return retval;
            }

        }
        return retval;
    }
}

