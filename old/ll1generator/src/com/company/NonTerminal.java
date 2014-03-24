package com.company;

/**
 * User: ruslanti
 * Date: 11/26/13
 */
public class NonTerminal implements Element {
    String nt;
    public NonTerminal(String name) {
       nt = name;
    }

    @Override
    public String toString() {
        return nt;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof NonTerminal)) return false;

        NonTerminal that = (NonTerminal) o;

        if (nt != null ? !nt.equals(that.nt) : that.nt != null) return false;

        return true;
    }

    @Override
    public int hashCode() {
        return nt != null ? nt.hashCode() : 0;
    }
}
