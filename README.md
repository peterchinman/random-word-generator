# random-word-generator
A Random Word Generator

```mermaid
---
title Dictionary
---
erDiagram
    Word||--|{Meaning : ""
    Meaning {
        int id pk
        string definition
        string example
        string speech_part
        int word_id fk
    }
    Meaning||--o{Synonym : ""
    Word{
        int id pk
        string word
    }
    Synonym{
        int id pk
        string synonym
        int meaning_id fk
    }
    Word||--|{Pronunciation : ""
    Pronunciation {
        int id pk
        string ARPAbet
        int word_id fk
    }
```
