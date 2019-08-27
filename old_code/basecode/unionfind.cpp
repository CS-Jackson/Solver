#include <vector>

using namespace std;

// 此处可使用路径压缩。比如子集A:a->b->c->d 
//通过路径压缩，直接让b、c、d 管 a叫爸爸。
int findParent(int x, vector<int>& parents) {
        if(x == parents[x])
            return x;
        else
            return findParent(parents[x], parents);
}
    
    
    // 将rank较低的子集合并到rank较高德子集,比如子集A:a->b->c  子集B: d,使子集B把子集A叫爸爸。
    // 当rank相等时，随意子集A和B谁叫谁爸爸都无所谓，并将其rank(儿子)+1
    void unions(int x, int y, vector<int>& parents,int& count, vector<int>& ranks)
    { 
        int xP = findParent(x, parents);
        int yP = findParent(y, parents);
        if(xP != yP) {
            if(ranks[xP] < ranks[yP]) 
                parents[xP] = yP;
            else if(ranks[xP] > ranks[yP]) 
                parents[yP] = xP;
            else
            {
                parents[xP] = yP;
                ranks[yP] += 1;
            }
            count--;
        }
    }
    int numIslands(vector<vector<char>>& grid) {
        if(grid.empty())
            return 0;
        int dx[4] = {-1, 0, 1, 0};
        int dy[4] = {0, -1, 0, 1};
        int row = grid.size();
        int col = grid[0].size();
        
        vector<int> parents(row * col,0);
        vector<int> ranks(row * col, 1);
        int count = 0;
        
        for(int r = 0; r < row; r++)
        {
            for(int c =0; c < col; c++)
            {
                if(grid[r][c] == '1')
                {
                    parents[r * col + c] = r * col + c;
                    count++;
                }    
            }
        }

        for(int r = 0; r < row; r++)
        {
            
            for(int c =0; c < col; c++)
            {

                if(grid[r][c] == '0')
                    continue;
                for(int k = 0; k < 4; ++k) {
                    int nr = r + dx[k];
                    int nc = c + dy[k];
                    if(nr < 0 || nr >= row || nc < 0 || nc >= col || grid[nr][nc] != '1') 
                        continue;
                    unions(r * col + c, nr * col + nc, parents,count, ranks);
 
                }
                
            }
        }
        return count;
    }
