package edu.duke.raft;

import java.util.Timer;
import java.util.TimerTask;
import java.util.*;

public class CandidateMode extends RaftMode {

  protected Timer t;
  protected boolean outdated = false;
  protected final static int ELECTION_TIMEOUT_T = 300;

  public void go () {
    synchronized (mLock) {
      mConfig.setCurrentTerm(mConfig.getCurrentTerm() + 1, mID);
      // System.out.println(mConfig);
      int term = mConfig.getCurrentTerm();  
      // int[] votes = RaftResponses.getVotes(); 
      RaftResponses.setTerm(term);
      System.out.println ("S" + 
			  mID + 
			  "." + 
			  term + 
			  ": switched to candidate mode.");
        // long timeout = mConfig.getTimeoutOverride();
        // if (timeout == 0){
        //   timeout = ((long)Math.random()*ELECTION_TIMEOUT_MIN + ELECTION_TIMEOUT_MIN); ;
        // } 
        // t = scheduleTimer(timeout,mID);
      long timeout = (long)(Math.random()*ELECTION_TIMEOUT_MIN + ELECTION_TIMEOUT_MIN);
      // System.out.println("The timeout (in new Candidate) for S"+mID+"."+term+" is "+(int)timeout); 
      t = scheduleTimer(timeout, mID);
      // System.out.println("S"+mID+"."+term+" is sending voteRPCS!");
      RaftResponses.clearVotes(term);
      // System.out.println("Initial votes after clear: "+Arrays.toString(RaftResponses.getVotes(term)));
      for (int i = 1; i <= mConfig.getNumServers(); i++) {
        if (i != mID) {
          remoteRequestVote(i,term,mID,mLog.getLastIndex(),mLog.getLastTerm());
        }
      } 
      
    }
  }

  // @param candidate’s term
  // @param candidate requesting vote
  // @param index of candidate’s last log entry
  // @param term of candidate’s last log entry
  // @return 0, if server votes for candidate; otherwise, server's
  // current term 
  public int requestVote (int candidateTerm,
			  int candidateID,
			  int lastLogIndex,
			  int lastLogTerm) {
    synchronized (mLock) {
      int term = mConfig.getCurrentTerm();
      int result = term;

      // t.cancel();
      // long timeout = (long)(Math.random()*ELECTION_TIMEOUT_MIN + ELECTION_TIMEOUT_MIN);
      // System.out.println("The timeout (in requestVote Candidate) for S"+mID+"."+term+" is "+(int)timeout); 
      // t = scheduleTimer(timeout, mID);
      // System.out.println("(Candidate) Vote request for S"+mID+"."+term+" from S"+candidateID+"."+candidateTerm);
      if (candidateTerm > term){
        if (lastLogTerm > mLog.getLastTerm() || (lastLogIndex >= mLog.getLastIndex() && lastLogTerm == mLog.getLastTerm())) {
          t.cancel();
          mConfig.setCurrentTerm(candidateTerm,candidateID);
          // System.out.println("S"+mID+"."+term+" is outdated! Switching to follower!");
          // outdated = true;
          RaftServerImpl.setMode(new FollowerMode());
          return 0;
          // outdated = true;
          // if (lastLogIndex >= mLog.getLastIndex() && lastLogTerm >= mLog.getLastTerm()) {
          //   mConfig.setCurrentTerm(candidateTerm,candidateID);
          //   // System.out.println(mConfig);
          //   System.out.println("S"+mID+"."+term+" is outdated! Setting outdated flag!");
          //   outdated = true;
          //   return 0;
          // }
        } else {
          t.cancel();
          mConfig.setCurrentTerm(candidateTerm,0);
          // System.out.println("S"+mID+"."+term+" is outdated! Switching to follower!");
          // outdated = true;
          RaftServerImpl.setMode(new FollowerMode());
          return mConfig.getCurrentTerm();
        }
      }
      return result;
    }
  }
  

  // @param leader’s term
  // @param current leader
  // @param index of log entry before entries to append
  // @param term of log entry before entries to append
  // @param entries to append (in order of 0 to append.length-1)
  // @param index of highest committed entry
  // @return 0, if server appended entries; otherwise, server's
  // current term
  public int appendEntries (int leaderTerm,
			    int leaderID,
			    int prevLogIndex,
			    int prevLogTerm,
			    Entry[] entries,
			    int leaderCommit) {
    synchronized (mLock) {
      int term = mConfig.getCurrentTerm();
      int result = term;

      // System.out.println("(Candidate) AppendEntries request for S"+mID+"."+term+" from S"+leaderID+"."+leaderTerm);
      // System.out.println("Candidate S"+mID+"."+term+" log: "+mLog);
      if(leaderTerm >= term) {
        t.cancel();
        mConfig.setCurrentTerm(leaderTerm,0);
        // System.out.println("We outdated!");
        if (entries != null) {
          // System.out.println("First append check(follower)");
          if (prevLogIndex > mLog.getLastIndex() || (!(prevLogIndex <= -1) && prevLogTerm != mLog.getEntry(prevLogIndex).term)){
            // mLog.insert(entries,prevLogIndex, prevLogTerm);
            // System.out.println("Append failed!(follower)");
            // return 0;
            RaftServerImpl.setMode(new FollowerMode());
            return mConfig.getCurrentTerm();
          }
          else{
            // System.out.println("We appendin'!(follower)");
            if (mLog.insert(entries,prevLogIndex,prevLogTerm) == -1) {
              RaftServerImpl.setMode(new FollowerMode());
              return mConfig.getCurrentTerm();
            }  
            mCommitIndex = leaderCommit;
            // System.out.println("Follower S"+mID+"."+term+" log: "+mLog);
            RaftServerImpl.setMode(new FollowerMode());
            return 0;
          }
        }
        else {
          RaftServerImpl.setMode(new FollowerMode());
          return 0;
        }
          
      }
      return term;
    }
  }

  // @param id of the timer that timed out
  public void handleTimeout (int timerID) {
    synchronized (mLock) {
      if (timerID == mID) {
        //Handle
        t.cancel();
        int term = mConfig.getCurrentTerm();
        // if (outdated) {
        //   // System.out.println("S"+mID+"."+term+" is outdated! Swiching to follower!");
        //   // RaftResponses.clearVotes(term);
        //   RaftServerImpl.setMode(new FollowerMode());
        // }
        // else {
        // System.out.println("S"+mID+"."+term+" is counting these votes!");
        int[] votes = RaftResponses.getVotes(term);
        
        if(votes != null) {
          int count = 1;

          // System.out.println("Vote count result: "+Arrays.toString(votes));

          for (int i = 1; i <= mConfig.getNumServers(); i++) {
            if(i != mID && votes[i] == 0) {
              count++;
            }
            else if (votes[i] > mConfig.getCurrentTerm()) {
              mConfig.setCurrentTerm(votes[i],mID);
              outdated = true;
              break;
            }
          }
          // System.out.println("S"+mID+"."+term+" attained "+count+" votes!");
          if(count > mConfig.getNumServers()/2 && !outdated) {
            RaftResponses.clearVotes(mConfig.getCurrentTerm());
            RaftServerImpl.setMode(new LeaderMode());
          }
          else if(!outdated) {
            go();
          }
        } else {
          go();
        }
        // go();
        // }
        // RaftServerImpl.setMode(new CandidateMode());
      }
      else {
        throw new RuntimeException("Oops I didn't set this timer!");
      }
    }
  }

  public long getTimeout() {
    return (long)(Math.random()*ELECTION_TIMEOUT_MIN + ELECTION_TIMEOUT_MIN);
  } 
}

 // LeaderMode lm = new LeaderMode();
            // lm.initializeServer(mConfig,mLog,mLastApplied,mRmiPort,mID);
            // mConfig.setCurrentTerm((mConfig.getCurrentTerm() + 1), mID);

                      // FollowerMode fm = new FollowerMode();
          // fm.initializeServer(mConfig,mLog,mLastApplied,mRmiPort,mID);
          // mConfig.setCurrentTerm((mConfig.getCurrentTerm() + 1), mID);

                // if (votes[mID] > mConfig.getNumServers()/2) {
      //     LeaderMode lm = new LeaderMode();
      //     lm.initializeServer(mConfig,mLog,mLastApplied,mRmiPort,mID);
      //     // mConfig.setCurrentTerm((mConfig.getCurrentTerm() + 1), mID);
      //     RaftServerImpl.setMode(lm);
      // }  

