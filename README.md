# random-word-generator
A Random Word Generator

```mermaid
---
title Dictionary
---
erDiagram
    Word||--|{Meanings : ""
    Meanings {
        int id pk
        string definition
        string example
        string speech_part
        int word_id fk
    }
    Meanings||--o{Synonyms : ""
    Word{
        int id pk
        string word
    }
    Synonyms{
        int id pk
        string synonym
        int meaning_id fk
    }
    Word||--|{Pronunciations : ""
    Pronunciations {
        int id pk
        string ARPAbet
        int word_id fk
    }
```
