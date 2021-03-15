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


    ######### common waf cluster 通用 waf 集群
    location = /waf_target_common {
        proxy_pass http://xxx.xxx.xxx.xxx$request_uri;
    }

    ######### heartbeat waf cluster 心跳 waf 集群
    location = /waf_target_heartbeat {
        proxy_pass http://yyy.yyy.yyy.yyy$request_uri;
    }
  }
}
```
```
在上面的配置中，login jpg jpeg 路径都会走 通用 waf 风控集群，heaetbet 路径

会走 心跳 waf 风控集群，50x.html 路径不走任何风控集群。
```
