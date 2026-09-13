XOMMIT uses **Groq** to turn your vague message into a commit message. It's free, you just need an API key. Takes like a minute.

1. Go to [console.groq.com](https://console.groq.com) and create an account. You can sign up with Google, GitHub or email, no card needed.
2. Once you're in, open the [API Keys](https://console.groq.com/keys) page from the sidebar.
3. Click **Create API Key**, give it any name (like `xommit`) and submit.
4. Copy the key it shows you. It starts with `gsk_` and **you only see it once**, so grab it right away. Lost it? Just delete it and make a new one.

Now install xommit below, then run this and paste the key when it asks:

```terminal
xommit --connect
```

You can re-run `xommit --connect` anytime to change the key, or wipe it with `xommit --reset`.
