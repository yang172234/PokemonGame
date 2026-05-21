const https = require('https')
const fs = require('fs')

const pokemonIds = {
  1: 25,   // 皮卡丘
  2: 6,    // 喷火龙
  3: 9,    // 水箭龟
  4: 3,    // 妙蛙花
  5: 150,  // 超梦
  6: 151,  // 梦幻
  7: 143,  // 卡比兽
  8: 149,  // 快龙
  9: 133   // 伊布
}

function downloadImage(id, pokemonId) {
  const url = `https://github.com/PokeAPI/sprites/blob/master/sprites/pokemon/other/official-artwork/${pokemonId}.png`
  const filePath = `public/images/pokemon_${id}.png`
  
  https.get(url, (response) => {
    const chunks = []
    response.on('data', (chunk) => chunks.push(chunk))
    response.on('end', () => {
      const buffer = Buffer.concat(chunks)
      fs.writeFileSync(filePath, buffer)
      console.log(`Downloaded: pokemon_${id}.png`)
    })
  }).on('error', (err) => {
    console.error(`Error downloading pokemon_${id}:`, err.message)
  })
}

// 创建目录
if (!fs.existsSync('public/images')) {
  fs.mkdirSync('public/images', { recursive: true })
}

// 下载所有图片
for (const [id, pokemonId] of Object.entries(pokemonIds)) {
  downloadImage(id, pokemonId)
}