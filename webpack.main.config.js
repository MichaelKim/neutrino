const path = require('path');
const TerserPlugin = require('terser-webpack-plugin');

const mainConfig = {
  entry: './src/main/index.js',
  output: {
    path: path.resolve('./dist/neutrino'),
    filename: 'main.js',
    library: 'main',
    chunkFormat: 'array-push'
  },
  target: 'es5',
  module: {
    rules: [
      {
        test: /\.js$/,
        exclude: /node_modules/,
        use: [
          {
            loader: 'babel-loader',
            options: {
              presets: ['@babel/preset-env']
            }
          }
        ]
      }
    ]
  },
  plugins: [],
  resolve: {
    alias: {
      fs: 'neutrinojs/lib/fs'
    }
  },
  node: {
    __dirname: false
  },
  stats: {
    colors: true
  }
};

module.exports = (env, argv) => {
  mainConfig.output.path = env.path;

  if (argv.mode === 'production') {
    mainConfig.mode = 'production';
    mainConfig.optimization = {
      minimizer: [
        new TerserPlugin({
          extractComments: false,
          terserOptions: {
            compress: {
              drop_console: true
            }
          }
        })
      ]
    };
  } else {
    mainConfig.mode = 'development';
    mainConfig.devtool = 'cheap-module-source-map';
  }

  return mainConfig;
};
