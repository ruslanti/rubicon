package com.company;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Stack;

public class Main {
    public static final int TERMINALS_NUM = 256;
    private Map<NonTerminal, List<List<Element>>> productions = new HashMap<NonTerminal, List<List<Element>>>();

    private Map<NonTerminal, Set<Terminal>> firsts;
    private Map<List<Element>, Set<Terminal>> firstsW;
    private Map<NonTerminal, Set<Terminal>> follows;

    private Set<NonTerminal> nonTerminals = new HashSet<NonTerminal>();
    private Set<Terminal> terminals = new HashSet<Terminal>();

    private List<NonTerminal> nonTerminalsIndex;
    //private List<Terminal> terminalsIndex;

    private List<Element>[][] table;

    private Stack<Element> stack;
    private NonTerminal root;
    private int pos;

    protected void load_productions(String file) throws IOException {

        BufferedReader bf = new BufferedReader(new FileReader(file));
        String line = null;
        while ((line = bf.readLine()) != null) {
            if (line.startsWith("#"))
                continue;
            int eqIndex = line.indexOf('=');
            if (eqIndex < 1)
                continue;
            String key = line.substring(0, eqIndex).trim();
            NonTerminal nt = new NonTerminal((String) key);
            List<List<Element>> production_rules = productions.get(nt);
            if (production_rules == null) {
                production_rules = new ArrayList<List<Element>>();
                productions.put(nt, production_rules);
                nonTerminals.add(nt);
            }
            String value = line.substring(eqIndex+1).trim();
            for (String elements : value.split("\\|")) {
                elements = elements.trim();
                List<Element> rule = new ArrayList<Element>();
                for (String element : elements.split("\\ ")) {
                    Element el;
                    if (element.length() == 3 && element.charAt(0) == '\"' && element.charAt(2) == '\"') {
                        el = new Terminal(element.charAt(1));
                    } else if ( element.charAt(0) == '\\') {
                        int i = Integer.parseInt(element.substring(1), 16);
                        el = new Terminal((char) i);
                    } else if (element.equals("EPSILON")) {
                        el = Terminal.ε;
                    } else if (element.equals("EOF")) {
                        el = Terminal.eof;
                    } else {
                        el = new NonTerminal(element);
                    }
                    rule.add(el);
                    if (el instanceof Terminal)   {
                        terminals.add((Terminal) el);
                    }
                }
                production_rules.add(rule);
            }
        }

        for (NonTerminal nonTerminal : productions.keySet()) {
            System.out.println(nonTerminal+" = "+productions.get(nonTerminal));
        }


        bf.close();
    }
    /*
        S -> α Y β       [where α, Y, β could be terminals or non-terminals]
        first(S) = first(α Y β)
        If α is a terminal
            first(α Y β) = α
        else
            calculate first(α)
        if first(α) contains ε (can contain others too)
                first(α Y β) = first(α) - {ε} U first(Y β)   [Note 1]
        else  // (if first(α) does not contain ε)
                first(α Y β) = first(α)

        Note 1: first(Y β) is recursively calculated in a similar manner. However
        if we get to first (β) and that does contain ε, then since β was the last
        non-terminal, that final ε is included in the first set. (we must
        look at follow sets in this case to decide whether to choose ε).
     */
    private Set<Terminal> first(NonTerminal nt) {
        //System.out.println("first("+nt+")");
        if (firsts.containsKey(nt)) {
            return firsts.get(nt);
        } else {
            Set<Terminal> term = new HashSet<Terminal>();
            List<List<Element>> list = productions.get(nt);
            if (list == null)
                throw new NullPointerException("no rule found for predicate "+nt);
            for (List<Element> rule : list) {
                Set<Terminal> terminals = new HashSet<Terminal>();
                Set<Terminal> t = first(rule);
                for (Terminal t1 : t) {
                    if (term.contains(t1)) {
                        System.err.println(nt+"-> "+term+" contains "+t1);
                        assert(term.contains(t1));
                    }
                    term.add(t1);
                    terminals.add(t1);
                }
                firstsW.put(rule, terminals);
                if (!(rule.size() == 1 && rule.get(0) instanceof Terminal)) {
                    System.out.println("FIRST(" + rule + ") = " + terminals);
                }
            }
            firsts.put(nt, term);

            return term;
        }
    }

    private Set<Terminal> first(List<?> rule) {
        //System.out.println("first("+rule+")");
        Set<Terminal> term = new HashSet<Terminal>();
        if (rule.size() > 0) {
            Object obj = rule.get(0);
            if (obj instanceof Terminal) {
                term.add((Terminal) obj);
            } else {
                Set<Terminal> t = first((NonTerminal) obj);
                if (t.contains(Terminal.ε)) {
                    term.addAll(t);
                    List<?> rule1 = rule.subList(1, rule.size());
                    if (!rule1.isEmpty()) {
                        term.remove(Terminal.ε);
                        Set<Terminal> tt = first(rule1);
                        term.addAll(tt);
                    }
                } else {
                    term.addAll(t);
                }
            }

        }
        return term;
    }


    /*
    1. For a rule of type:
    A -> α X β
         where α, β are any sequence of terminals & non-terminals

    follow (X) = first (β) if first(β) does not contain ε
                 else //if first(B) contains ε
                 = first(β) - ε U follow(A)  [note 1]

    Note 1. This is because if β contains ε, then whatever follows
    A will follow β (since all of β can become an empty string)

    2. Rules of type:
        X -> a
        X -> abc
        X -> ε
    do not add anything to the follow set of X itself.

    3. For a rule of type:
        A -> α X       (X is the last item in the rule)

    follow (X) = follow (A)    [note 2]

    Note 2. Because whatever follows X in some rule will also follow A.

    Note: ε is never in a follow set. (but EOF (end of file) may be).
    The entire point of follow sets is to figure out when to choose
    A -> ε for the current non-terminal A (follow sets are useful only
    for non-terminals whose right hand side can go to ε). If ε is also in
    the follow set, then that would mean the current non-terminal should
    always go to ε.

     */
    protected Set<Terminal> follow(NonTerminal nt) {
        Set<Terminal> terminals = follows.get(nt);
        if (terminals == null) {
            terminals = new HashSet<Terminal>();
            follows.put(nt, terminals);
        }

        for (Map.Entry<NonTerminal, List<List<Element>>> nonTerminalListEntry : productions.entrySet()) {
            NonTerminal key = nonTerminalListEntry.getKey();
            if (key.equals(nt))
                continue;
            List<List<Element>> rules = nonTerminalListEntry.getValue();
            for (List<Element> rule : rules) {
                int indexOfNt = rule.indexOf(nt);
                if (indexOfNt > -1) {
                    if (indexOfNt == (rule.size()-1)) {
                        // rule 3. last item in the rule
                        terminals.addAll(follow(key));
                    } else {
                        List<Element> beta = rule.subList(indexOfNt+1, rule.size());
                        Set<Terminal> firstBeta = first(beta);
                        if (!firstBeta.contains(Terminal.ε)) {
                            terminals.addAll(firstBeta);
                        } else {
                            for (Terminal terminal : firstBeta) {
                                if (terminal != Terminal.ε)
                                    terminals.add(terminal);
                            }
                            terminals.addAll(follow(key));
                        }
                    }
                }
            }
        }
        return terminals;
    }

    protected void init_follow() {
        follows = new HashMap<NonTerminal, Set<Terminal>>();
        for (NonTerminal nt : nonTerminals) {
            System.out.println("FOLLOW(" + nt + ") = " + follow(nt));
        }
    }

    protected void init_first() {
        firsts = new HashMap<NonTerminal, Set<Terminal>>();
        firstsW = new HashMap<List<Element>, Set<Terminal>>();
        //for (int i = 0; i < 10; i++)
        for (NonTerminal nt : productions.keySet()) {
            //System.out.println(nt + "->" + first(nt));
            first(nt);
        }
    }

    protected void init_table() {

        table = new List[nonTerminals.size()][TERMINALS_NUM];

        nonTerminalsIndex = new ArrayList<NonTerminal>(nonTerminals);
        //terminalsIndex = new ArrayList<Terminal>(terminals);

        for (Map.Entry<NonTerminal, List<List<Element>>> nonTerminalListEntry : productions.entrySet()) {
            NonTerminal nt = nonTerminalListEntry.getKey();
            int nonTerminalIndex = nonTerminalsIndex.indexOf(nt);
            if (nonTerminalIndex < 0) {
                System.err.println("Index "+nonTerminalIndex +" for "+nt);
                System.err.println(nonTerminalsIndex);
            }
            List<List<Element>> rules = nonTerminalListEntry.getValue();
            for (List<Element> rule : rules) {
                if (firstsW.get(rule) != null) {
                    List<Element> elements = new ArrayList<Element>(rule.size());
                    for (int j = rule.size() -1; j >= 0; j--) {
                        Element item = rule.get(j);
                        elements.add(item);
                    }
                    for (Terminal terminal : firstsW.get(rule)) {
                        if ((terminal == Terminal.ε)) {
                            for (Terminal terminal1 : follows.get(nt)) {
                                table[nonTerminalIndex][terminal1.order()] = elements;
                            }
                        } else {
                            table[nonTerminalIndex][terminal.order()] = elements;
                        }
                    }
                }
            }
        }
      /*  System.out.print("\t");
        for (int i = 0; i < TERMINALS_NUM; i++) {
            System.out.print("'"+(char)i+"'\t");
        }
        System.out.println("");
        for (NonTerminal nonTerminal : nonTerminalsIndex) {
            System.out.print(nonTerminal);
            for (int i = 0; i < TERMINALS_NUM; i++) {
                List<Element> rule = table[nonTerminalsIndex.indexOf(nonTerminal)][i];
                if (rule != null)
                    System.out.print("\t"+ rule);
                else
                    System.out.print("\tnull");
            }
            System.out.println("");
        }*/
    }

    private void save_table_header(String name) {
        StringBuffer buf = new StringBuffer();
        buf.append("#ifndef ").append(name).append("_H_\n");
        buf.append("#define ").append(name).append("_H_\n\n");

        buf.append("#define NON_TERMINAL_DENOM\t").append('0').append(Integer.toOctalString(TERMINALS_NUM)).append("\n");
        buf.append("#define E\t").append('0').append(Integer.toOctalString(Terminal.ε.order())).append("\n");
        buf.append("#define NON_TERMINALS\t").append(nonTerminalsIndex.size()).append('\n');
        buf.append("#define TERMINALS\t").append(TERMINALS_NUM).append("\n\n");

        buf.append("int *parsing_table_").append(name).append('[').append("NON_TERMINALS").append(']').append('[').append("TERMINALS").append(']');
        buf.append(" = {\n");

        for (NonTerminal nonTerminal : nonTerminalsIndex) {
            buf.append('\t').append(" /* ").append("0").append(Integer.toOctalString(TERMINALS_NUM + nonTerminalsIndex.indexOf(nonTerminal))).append(" */ ").append('{');
            for (int t = 0; t < TERMINALS_NUM; t++) {
                List<Element> rule = table[nonTerminalsIndex.indexOf(nonTerminal)][t];
                if (rule != null) {
                    buf.append('{');
                    for (Element element : rule) {
                        if (element instanceof Terminal) {
                            if (element.equals(Terminal.ε))
                                buf.append("E").append(',');
                            else
                                buf.append("0").append(Integer.toOctalString(((Terminal) element).order())).append(',');
                        } else
                            buf.append("0").append(Integer.toOctalString(TERMINALS_NUM + nonTerminalsIndex.indexOf(element))).append(',');
                    }
                    buf.append(0).append('}');
                } else {
                    buf.append('{').append(0).append('}');
                }
                buf.append(',');
            }
            buf.append('}').append(',').append(" /* ").append(nonTerminal).append(" */ ").append('\n');
        }

        buf.append("};\n\n");
        buf.append("#endif /* ").append(name).append("_H_ */\n");

        System.out.println(buf.toString());
    }

    public void init_stack(String rootElemenetName) {
        stack = new Stack<Element>();
        stack.push(new NonTerminal(rootElemenetName));
        pos = 0;
    }

    public boolean parse(String text) {
        if (text.isEmpty())
            return false;
        int i = 0;

        while(!stack.isEmpty() && pos < text.length()) {
            //System.out.println(text.substring(pos) + " "+(++i)+"  " + stack);

            Terminal terminal = new Terminal(text.charAt(pos));

            Element element = stack.pop();

            if (element instanceof Terminal & terminal.equals(element)) {
                pos++;
                continue;
            }

            List<Element> rule = table[nonTerminalsIndex.indexOf(element)][terminal.order()];

            if (rule != null && !rule.isEmpty()) {
                for (Element item : rule) {
                    if (!item.equals(Terminal.ε))
                        stack.push(item);
                }
            } else {
                System.err.println("parse error "+element+" - <"+terminal+">");
                return false;
            }
        }
        System.out.println("parsed "+pos+" chars from "+text.length());
        return true;
    }

    public static final void main(String[] args) throws IOException {
/*
        for (int i = 32; i<256; i++) {
*//*            if (i == '(' || i == ')' || i == '<' || i ==  '>' || i ==  '@' || i ==  ',' || i ==  ';' || i ==  ':'
                    || i ==  '\\' || i ==  '\"' || i ==  '/' || i ==  '[' || i ==  ']' || i ==  '?' || i ==  '='
                    || i ==  '{' || i ==  '}' || i ==  ' ' || i == '\t')
                continue;*//*
            System.out.print("\\" + Integer.toString(i, 16) + " | ");
        }
        System.out.println();*/

        Main main = new Main();
        System.out.println("------------------------ Production rules:");
        main.load_productions("/home/ruslanti/workspace/ngnms/untitled/rules.txt");
        System.out.println("------------------------ Firsts:");
        main.init_first();
        System.out.println("------------------------ Follows:");
        main.init_follow();
        System.out.println("------------------------ Table:");
        main.init_table();
        main.save_table_header("HTTP");

/*        main.parse("http://www.yahoo.com//path1/path2?search&a=1$");
        main.parse("http://localhost");*/
        String http = "HEAD / HTTP/1.1\r\n" +
                "User-Agent: Opera/9.80 (X11; Linux x86_64) Presto/2.12.388 Version/12.15\r\n" +
                "Host: www.linux.org.ru\r\n" +
                "Accept: text/html, application/xml;q=0.9, application/xhtml+xml, image/png, image/webp, image/jpeg, image/gif, image/x-xbitmap, */*;q=0.1\r\n" +
                "Accept-Language: en-US,en;q=0.9\r\n" +
                "Accept-Encoding: gzip, deflate\r\n" +
                "If-Modified-Since: Wed, 04 Dec 2013 07:27:17 GMT\r\n" +
                "Cookie: CSRF_TOKEN=\"usQqYy8erfohfCYA7mEBTg==\"; JSESSIONID=75F17533DBF2E6AC95EDAF2A1F6434E8; __utma=75309071.158983938.1366621658.1386142040.1386145670.617; __utmb=75309071.1.10.1386145670; __utmc=75309071; __utmz=75309071.1371534575.157.2.utmcsr=google|utmccn=(organic)|utmcmd=organic|utmctr=(not%20provided)\r\n" +
                "Connection: Keep-Alive\r\n" +
                "\r\n";

        main.init_stack("http-message");
        if (!main.parse(http))
            System.exit(-1);


        String response = "HTTP/1.1 200 OK\r\n" +
                "Server: Apache-Coyote/1.1\r\n" +
                "Cache-Control: private\r\n" +
                "Expires: Tue, 03 Dec 2013 16:39:23 GMT\r\n" +
                "Last-Modified: Wed, 04 Dec 2013 12:39:21 GMT\r\n" +
                "Content-Type: text/html;charset=utf-8\r\n" +
                "Content-Language: en-US\r\n" +
                "Transfer-Encoding: chunked\r\n" +
                "Content-Encoding: gzip\r\n" +
                "Vary: Accept-Encoding\r\n" +
                "Date: Wed, 04 Dec 2013 12:39:23 GMT\r\n" +
                "\r\n";

        main.init_stack("http-message");
        if (!main.parse(response))
            System.exit(-1);

        String ssdp = "M-SEARCH * HTTP/1.1\r\n" +
                "Host:239.255.255.250:1900\r\n" +
                "ST:urn:schemas-upnp-org:device:InternetGatewayDevice:1\r\n" +
                "Man:\"ssdp:discover\"\r\n" +
                "MX:3\r\n" +
                "\r\n";

        main.init_stack("http-message");
        if (!main.parse(ssdp))
            System.exit(-1);


        String get = "GET /subscribe?host_int=585906152&ns_map=313743555_116277860547,216893388_24408516036556,225340374_373887495126,303252655_163512009903&user_id=134857949&nid=1383209876696916225&ts=1386160765 HTTP/1.1\r\n" +
                "Host: notify10.dropbox.com\r\n" +
                "Accept-Encoding: identity\r\n" +
                "Connection: keep-alive\r\n" +
                "X-Dropbox-Locale: en_US\r\n" +
                "User-Agent: DropboxDesktopClient/2.0.22 (Linux; 3.11.6-200.fc19.x86_64; x64; en_US)\r\n" +
                "\r\n";

        main.init_stack("http-message");
        if (!main.parse(get))
            System.exit(-1);

    }

}
