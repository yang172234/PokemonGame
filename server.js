const express = require('express');
const { exec } = require('child_process');
const cors = require('cors');
const app = express();
app.use(cors());
app.use(express.json());
app.use(express.static('.'));

// 初始化
app.post('/api/init', (req, res) => {
    exec(`pokemon.exe init ${req.body.pid}`, (err, stdout) => {
        res.send(stdout);
    });
});

// 战斗
app.post('/api/fight', (req, res) => {
    const { pid, skill, php, ehp } = req.body;
    exec(`pokemon.exe fight ${pid} ${skill} ${php} ${ehp}`, (err, stdout) => {
        res.send(stdout);
    });
});

app.listen(3000, () => {
    console.log('测试页面：http://localhost:3000');
});