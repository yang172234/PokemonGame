const https = require('https')
const fs = require('fs')

const pokemonMap = {
  1: { id: 25, name: 'pikachu' },
  2: { id: 6, name: 'charizard' },
  3: { id: 9, name: 'blastoise' },
  4: { id: 3, name: 'venusaur' },
  5: { id: 150, name: 'mewtwo' },
  6: { id: 151, name: 'mew' },
  7: { id: 143, name: 'snorlax' },
  8: { id: 149, name: 'dragonite' },
  9: { id: 133, name: 'eevee' }
}

// 备选图片源列表（按优先级排序）
const imageSources = [
  (id, name) => `https://raw.githubusercontent.com/PokeAPI/sprites/master/sprites/pokemon/other/official-artwork/${id}.png`,
  (id, name) => `https://raw.githubusercontent.com/PokeAPI/sprites/master/sprites/pokemon/other/dream-world/${id}.svg`,
  (id, name) => `https://chiy.uk/full/${name}`,
  (id, name) => `https://raw.githubusercontent.com/PokeAPI/sprites/master/sprites/pokemon/${id}.png`  // 默认小图
]

function downloadImage(url, filepath) {
  return new Promise((resolve, reject) => {
    https.get(url, (response) => {
      if (response.statusCode === 200) {
        const chunks = []
        response.on('data', chunk => chunks.push(chunk))
        response.on('end', () => {
          fs.writeFileSync(filepath, Buffer.concat(chunks))
          console.log(`✓ 下载成功: ${filepath}`)
          resolve(true)
        })
      } else {
        console.log(`✗ 失败 (${response.statusCode}): ${url}`)
        resolve(false)
      }
    }).on('error', (err) => {
      console.log(`✗ 错误: ${url} - ${err.message}`)
      resolve(false)
    })
  })
}

async function downloadAll() {
  if (!fs.existsSync('public/images')) {
    fs.mkdirSync('public/images', { recursive: true })
  }

  for (const [localId, info] of Object.entries(pokemonMap)) {
    let success = false
    for (const getUrl of imageSources) {
      const url = getUrl(info.id, info.name)
      const ext = url.includes('.svg') ? '.svg' : '.png'
      const filepath = `public/images/pokemon_${localId}${ext}`
      
      success = await downloadImage(url, filepath)
      if (success) break
    }
    
    if (!success) {
      console.log(`⚠ 精灵 ${info.name} 所有源都失败，请手动添加图片`)
    }
  }
}

downloadAll()