const path = require('path');
const webpack = require('webpack');
const TerserPlugin = require('terser-webpack-plugin');

const mainConfig = {
  entry: './src/main/index.js',
  output: {
    filename: 'main.js',
    library: 'main'
  },
  module: {
    rules: [
      {
        test: /\.jsx?$/,
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
    },
    extensions: ['.js', '.jsx']
  },
  node: {
    fs: 'empty',
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
    mainConfig.plugins.push(
      new webpack.DefinePlugin({
        'process.env.PRODUCTION': JSON.stringify(true)
      })
    );
  } else {
    mainConfig.mode = 'development';
    mainConfig.devtool = '#cheap-module-source-map';
    mainConfig.plugins.push(
      new webpack.DefinePlugin({
        'process.env.PRODUCTION': JSON.stringify(false)
      })
    );
  }

  return mainConfig;
};
