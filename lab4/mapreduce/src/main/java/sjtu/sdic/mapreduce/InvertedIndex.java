package sjtu.sdic.mapreduce;

import org.apache.commons.io.filefilter.WildcardFileFilter;
import sjtu.sdic.mapreduce.common.KeyValue;
import sjtu.sdic.mapreduce.core.Master;
import sjtu.sdic.mapreduce.core.Worker;

import java.io.File;
import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Created by Cachhe on 2019/4/24.
 */
public class InvertedIndex {

    public static List<KeyValue> mapFunc(String file, String value) {
        // Your code here (Part V)
        Pattern compile = Pattern.compile("[a-zA-Z0-9]+");
        Matcher matcher = compile.matcher(value);

        List<KeyValue> kvs = new ArrayList<>();
        while(matcher.find()) {
            kvs.add(new KeyValue(matcher.group(),file));
        }
        return kvs;
    }

    public static String reduceFunc(String key, String[] values) {
        //  Your code here (Part V)
        Set<String> distinctValues = new HashSet<>();
        for(String s : values) {
            if(!distinctValues.contains(s)) {
                distinctValues.add(s);
            }
        }
        StringBuilder sb = new StringBuilder(" ");
        for(String s : distinctValues) {
            sb.append(s);
            sb.append(',');
        }
        sb.insert(0, distinctValues.size());
        sb.deleteCharAt(sb.lastIndexOf(","));
        return sb.toString();
    }

    public static void main(String[] args) {
        if (args.length < 3) {
            System.out.println("error: see usage comments in file");
        } else if (args[0].equals("master")) {
            Master mr;

            String src = args[2];
            File file = new File(".");
            String[] files = file.list(new WildcardFileFilter(src));
            if (args[1].equals("sequential")) {
                mr = Master.sequential("iiseq", files, 3, InvertedIndex::mapFunc, InvertedIndex::reduceFunc);
            } else {
                mr = Master.distributed("wcdis", files, 3, args[1]);
            }
            mr.mWait();
        } else {
            Worker.runWorker(args[1], args[2], InvertedIndex::mapFunc, InvertedIndex::reduceFunc, 100, null);
        }
    }
}
