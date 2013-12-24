package com.company;

/**
 * User: ruslanti
 * Date: 11/26/13
 */
public class Terminal implements Element {

    public final static Terminal ε = new Terminal();
    public final static Terminal eof = new Terminal('$');

    char ch;
    boolean epsilon = false;

    public Terminal(char ch) {
        this.ch = ch;
    }

    private Terminal() {
        ch = (char) -1;
        epsilon = true;
    }

    public int order() {
        return (int)ch;
    }

    @Override
    public String toString() {
        if (epsilon)
            return "ε";
        else if (ch == '\r')
            return "\"\\r\"";
        else if (ch == '\n')
            return "\"\\n\"";
        else
            return "\"" + ch + '\"';
    }

    public boolean isEpsilon() {
        return epsilon;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof Terminal)) return false;

        Terminal terminal = (Terminal) o;

        if (ch != terminal.ch) return false;
        if (epsilon != terminal.epsilon) return false;

        return true;
    }

    @Override
    public int hashCode() {
        int result = (int) ch;
        result = 31 * result + (epsilon ? 1 : 0);
        return result;
    }
}
