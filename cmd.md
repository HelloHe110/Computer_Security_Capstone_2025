### 1 Build Image

Run the following command to setup the image for development:

```bash
docker compose build
```

## 2. Run Containers

### 2.1 Bring up the Environment

Run the attacker and victim with

```bash
docker compose up -d
```

### 2.2 Attach to the Container

Attach to container with:

```bash
docker exec -it container_name bash
```

### 3. Restart the Container

If the container exited after rebooting,
restart the container with

```bash
$ docker restart <container_name>
```

### 4. Stop the ContainerStop and remove the containers
Remove the docker network and containers with

```
docker compose down
```

### 5. Remove the Images

Remove the docker image (csc2025-project3) with

```
docker rmi project4-password_checker project4-secure_random project4-simple_shell project4-simple_rop project4-ret2flag project4-simple_rtos project4-hard_rop
```

