package sjtu.sdic.mapreduce.core;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import sjtu.sdic.mapreduce.common.KeyValue;
import sjtu.sdic.mapreduce.common.Utils;

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.util.*;

/**
 * Created by Cachhe on 2019/4/19.
 */
public class Reducer {

    /**
     * 
     * 	doReduce manages one reduce task: it should read the intermediate
     * 	files for the task, sort the intermediate key/value pairs by key,
     * 	call the user-defined reduce function {@code reduceF} for each key,
     * 	and write reduceF's output to disk.
     * 	
     * 	You'll need to read one intermediate file from each map task;
     * 	{@code reduceName(jobName, m, reduceTask)} yields the file
     * 	name from map task m.
     *
     * 	Your {@code doMap()} encoded the key/value pairs in the intermediate
     * 	files, so you will need to decode them. If you used JSON, you can refer
     * 	to related docs to know how to decode.
     * 	
     *  In the original paper, sorting is optional but helpful. Here you are
     *  also required to do sorting. Lib is allowed.
     * 	
     * 	{@code reduceF()} is the application's reduce function. You should
     * 	call it once per distinct key, with a slice of all the values
     * 	for that key. {@code reduceF()} returns the reduced value for that
     * 	key.
     * 	
     * 	You should write the reduce output as JSON encoded KeyValue
     * 	objects to the file named outFile. We require you to use JSON
     * 	because that is what the merger than combines the output
     * 	from all the reduce tasks expects. There is nothing special about
     * 	JSON -- it is just the marshalling format we chose to use.
     * 	
     * 	Your code here (Part I).
     * 	
     * 	
     * @param jobName the name of the whole MapReduce job
     * @param reduceTask which reduce task this is
     * @param outFile write the output here
     * @param nMap the number of map tasks that were run ("M" in the paper)
     * @param reduceF user-defined reduce function
     */
    public static void doReduce(String jobName, int reduceTask, String outFile, int nMap, ReduceFunc reduceF) {
        try {
            // read from each intermediate file and combine distinct key
            Map<String, List<String>> keyValues = new HashMap<>();
            for (int m = 0; m < nMap; m++) {
                // read file and parse JSON to array
                InputStream inputStream = new FileInputStream(Utils.reduceName(jobName, m, reduceTask));
                byte[] content = new byte[inputStream.available()];
                inputStream.read(content);
                List<KeyValue> kvContent = JSON.parseArray(new String(content), KeyValue.class);
                // traverse array to fill map
                for(KeyValue kv : kvContent) {
                    if(keyValues.containsKey(kv.key)) {
                        keyValues.get(kv.key).add(kv.value);
                    }
                    else {
                        List<String> values = new ArrayList<>();
                        values.add(kv.value);
                        keyValues.put(kv.key,values);
                    }
                }
                inputStream.close();
            }
            // do reduce on each distinct key and write to target file
            JSONObject jsonObject = new JSONObject();
            for(Map.Entry<String, List<String>> entry : keyValues.entrySet()) {
                String value = reduceF.reduce(entry.getKey(), entry.getValue().toArray(new String[0]));
                jsonObject.put(entry.getKey(), value);
            }
            OutputStream outputStream = new FileOutputStream(outFile);
            outputStream.write(jsonObject.toString().getBytes());
            outputStream.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
