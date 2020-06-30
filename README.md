nopasswd-scan
=============

Find files that don't need a password to be executed by sudo.

Usually, this can be done by using ```sudo -l```, if the security policy (```listpw```) isn't set to ask a password, if this isn't your case, this tool will help you.


Usage
-----

```
$ ./nopasswd-scan [CMDLIST]
```

**[CMDLIST]** is the file that contains the command list that will be checked.

The list format is simple, any escape character will be treated as a new parameter, if you want a space character instead of a new parameter, you must use backslash as an escape character. You should use a double backslash if you want a backslash, and if you need binary characters, you can use hex escape (e.g. ```\xff```). If an invalid escape sequence is given, such as a hex escape with invalid hex chars, then this will be ignored and the characters won't be replaced.

**Examples:**

Suppose that the cmd list has the following line:  
```/my/prog I\ want\ this\ as\ a\ single parameter```

The program will execute:  
```sudo /my/prog/ "I want this as a single parameter"```

> The same result can be achieved using hex escape:  
> ```/my/prog I\x20want\x20this\x20as\x20a\x20single\x20parameter```

Now suppose that the cmd list has this line:  
```/my/prog "I want this as a single parameter"```

The program will execute:  
```sudo /my/prog "\"I" "want" "this" "as" "a" "single" "parameter\""```

Demo
----

![](https://raw.githubusercontent.com/hc0d3r/nopasswd-scan/demo/demo.gif)


Contributing
------------
You can help with code, or donating money.
If you wanna help with code, use the kernel code style as a reference.

Paypal: [![](https://www.paypalobjects.com/en_US/i/btn/btn_donate_SM.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=RAG26EKAYHQSY&currency_code=BRL&source=url)

BTC: 1PpbrY6j1HNPF7fS2LhG9SF2wtyK98GSwq
