## BackGround

```
Nginx Web Application Firewall (WAF) Module
```

## Syntax

```
             Syntax:	waf_target uri | off;
             Default:	waf_target off;
             Context:	http, server, location
```

## Conf

```
http {

  server {

    waf_target /waf_target_common;

    location = /login {
    }
    
    location ~* .(jpg|jpeg)$ {
    }

    location = /heartbeat {

        waf_target /waf_tartget_heartbeat;
    }

    location = /50x.html {

        waf_target off;
    }


    ######### common waf cluster
    location = /waf_target_common {
        proxy_pass http://xxx.xxx.xxx.xxx$request_uri;
    }

    ######### heartbeat waf cluster
    location = /waf_target_heartbeat {
        proxy_pass http://yyy.yyy.yyy.yyy$request_uri;
    }
  }
}
```
```
login jpg jpeg will be targeted to common waf cluster

heaetbet will be targeted to heartbeat waf cluster

50x.html will not be targeted to any waf cluster
```

## Wiki

[Wiki](https://github.com/11061055/nginx_http_waf_module/wiki)
